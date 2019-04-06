#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define T_VAR 1

struct parser;

typedef struct {
	uint8_t type;
	char*   value;
} token;

typedef struct {
	uint64_t val;
	uint8_t  len;
} bitstring;

typedef token (*semantic_code) (struct parser*);

struct token_desc {
	int           lbp;
	semantic_code nud;
	semantic_code led;
};

struct parser {
	uint8_t* input;
	token left;
	token self;
};

static struct token_desc tokens[256];
static token parse (struct parser* p, int rbp);
static token vars[256];

static int lcm(int a, int b){
	int gcd = 1;

	for(int i = 1; i <= a && i <= b; ++i){
		if(a % i == 0 && b % i == 0)
			gcd = i;
	}

	return (a*b)/gcd;
}

static bitstring to_bitstring(token t){
	if(t.type != T_VAR)
		return (bitstring){};

	return (bitstring){
		.val = strtoul(t.value, NULL, 2),
		.len = strlen(t.value),
	};
}

static token boole(uint8_t m, token _x, token _y) {
	bitstring x = to_bitstring(_x), y = to_bitstring(_y);

	int len = lcm(x.len, y.len);

	token ret = {
		.type = T_VAR,
		.value = calloc(len + 1, 1),
	};

	for(int i = 0; i < len; ++i){
		_Bool a = (x.val & (1 << (i%x.len)));
		_Bool b = (y.val & (1 << (i%y.len)));
		ret.value[i] = (m & (1 << (a*2+b))) ? '1' : '0';
	}

	return ret;
}

static _Bool isvalid(token _t) {
	bitstring t = to_bitstring(_t);
	int cmp = (1 << t.len) - 1;
	return (t.val & cmp) == cmp;
}

static token generate(void) {
	static int k = 1;

	token t = {
		.type = T_VAR,
		.value = calloc(k*2+1, 1),
	};

	memset(t.value + 0, '0', k);
	memset(t.value + k, '1', k);

	k <<= 1;

	return t;
}

static token nonud(struct parser* p){
	if(tokens[p->self.type].led == NULL){
		token t = vars[p->self.type];
		if(t.value)
			return t;
		else {
			t = generate();
			vars[p->self.type] = t;
			return t;
		}
	} else {
		printf(" '%c' has no argument.\n", p->self.type);
		exit(1);
	}
}

static token led_question(struct parser* p){
	if(isvalid(p->left)){
		puts(" theorem");
	} else {
		puts(" non-theorem");
	}
	return parse(p, 1);
}

static token nud_paren(struct parser* p){
	token t = parse(p, 0);
	if(p->self.type != ')'){
		puts(" missing ')'");
		exit(1);
	} else {
		p->input++;
	}
	return t;
}

static token led_implies(struct parser* p){
	return boole(0b1101, p->left, parse(p, 1));
}

static token led_or(struct parser* p){
	return boole(0b1110, p->left, parse(p, 3));
}

static token led_and(struct parser* p){
	return boole(0b1000, p->left, parse(p, 4));
}

static token nud_not(struct parser* p){
	return boole(0b0101, parse(p, 5), (token){ T_VAR, strdup("0") });
}

static struct token_desc tokens[256] = {
	['?'] = { 1, NULL     , led_question },
	['('] = { 0, nud_paren, NULL         },
	[')'] = { 0, NULL     , NULL         },
	['>'] = { 2, NULL     , led_implies  },
	['v'] = { 3, NULL     , led_or       },
	['^'] = { 4, NULL     , led_and      },
	['~'] = { 0, nud_not  , NULL         },
};

static token parse(struct parser* p, int rbp){
	if(*p->input == 0)
		return (token){};

	uint8_t c = *p->input++;
	p->self = (token){ c };

	struct token_desc* d = tokens + c;
	if(d->nud){
		p->left = d->nud(p);
	} else {
		p->left = nonud(p);
	}

	for(;;){
		c = *p->input;
		p->self = (token){ c };
		d = tokens + c;

		if(c == 0)
			return (token){};

		if(rbp >= d->lbp)
			break;

		p->input++;

		if(d->led)
			p->left = d->led(p);
	}

	return p->left;
}

int main(void){
	char buf[256];
	if(!fgets(buf, sizeof(buf), stdin))
		return 1;

	printf("Input: %s", buf);

	struct parser p = {
		.input = (uint8_t*)buf,
	};

	parse(&p, 0);

	return 0;
}
