#ifndef SCANNER_DEF
#define SCANNER_DEF
#define CRT_SECURE_NO_WARNINGS
#define ID_LENGTH 12
#define NUMKEYWORD 7
#define NO_KEYWORDS  7  

struct tokentype {
	int number; // token number
	union {
		char id[ID_LENGTH];
		int num;
	}value; // token value
	char *tokenValue;
};
enum tsymbol {
	tnull = -1,
	tnot, tnotequ, tmod, tmodAssign, tident,
	tnumber, tand, tlparen, trparen, tmul,
	tmulAssign, tplus, tinc, taddAssign,
	tcomma, tminus, tdec, tsubAssign,
	tdiv, tdivAssign, tsemicolon, tless, tlesse,
	tassign, tequal, tgreat, tgreate, tlbracket,
	trbracket, teof, tconst, telse,
	tif, tint, treturn, tvoid, twhile,
	tlbrace, tor, trbrace
};
struct tokentype scanner();

#endif // !SCANNER_DEF
