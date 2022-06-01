/* CheapBasic: a tiny BASIC interpreter.
   Last updated Sep 12 2021.
   Copyright (C) 1992-2021 Takakazu NAKAMURA */

/*
#define	DEBUG
#define	TRACE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef  DEFINT // Maybe, int-type will be unsuported.
#define NUMBER            int
#define NUMBERFORMAT      "%d"
#define INPUTNUMBERFORMAT "%d"
#else
#define REAL
#define NUMBER            double
#define NUMBERFORMAT      "%.12g"
#define INPUTNUMBERFORMAT "%lf"
#endif

#define IDENTSYM      'a'
#define NUMSYM        'b'
#define STRINGSYM     'c'

#define PRINTSYM      'd'
#define INPUTSYM      'e'
#define IFSYM         'f'
#define THENSYM       'g'
#define LABELSYM      'h'
#define GOTOSYM       'i'
#define GOSUBSYM      'j'
#define RETURNSYM     'k'
#define ENDSYM        'l'
#define ANDSYM        'm'
#define ORSYM         'n'
#define NOTSYM        'o'
#define MODSYM        'p'
#define FORSYM        'q'
#define TOSYM         'r'
#define NEXTSYM       's'
#define STEPSYM       't'
#define INTSYM        'u'

#define MAXVARIABLES   5000
#define MAXNAMESIZE      31
#define MAXSOURCESIZE 10000
#define MAXSTRSIZE     1023

#define HASH_SEEK    0
#define HASH_GET     1
#define HASH_INSERT  2

#define ISALPHA(X) (('A' <= (X) && (X) <= 'Z') || ('a' <= (X) && (X) <= 'z'))
#define ISNUMERIC(X) ('0' <= (X) && (X) <= '9')
#define ISSPACE(X) ((0 < X) && (X <= 0x20) && (X != '\n'))
#define CHKERR(B, STR) { if(B)error((STR)); }
#define RESERVE(STR,CHR) { getvariable((STR),HASH_INSERT)->sym = (CHR); }

#define expression andor
NUMBER expression(int pre); /* pre-definiton */

typedef struct varrectag
{
	char name[MAXNAMESIZE + 1];
	union {
		NUMBER value;
		char *ptr;
	};
	int sym;
	struct varrectag *next;
}
varrec;

/*********************************************************************/
/* variables */
static varrec v_area[MAXVARIABLES];
char str[MAXSTRSIZE + 1], ch, sym;
int gsptr, fsptr ;
NUMBER num;
char *pc, *orgpc;


/*********************************************************************/
/* misc */
void error(const char *errstr)
{
	char *k = orgpc;
	int line_no = 1;
	char *current_line = orgpc;

	if (*(pc - 2) == '\n')
		pc--; // [suspend] not so good...^_^;
	for (k = orgpc ; k <pc-1 ; k++) {
		if (*k == '\n') {
			line_no++;
			current_line = k+1;
		}
	}
	*(pc-1) = '\0';
	printf("\n%6d: %s <<<< %s.\n", line_no, current_line, errstr);
#ifdef DEBUG
	printf ("pc = %p sym = %d('%c') str ='%s'.\n",pc, sym, sym, str);
#endif
	exit(1);
}

char *safecat(char *to, const char *from)
{
	if (strlen(to) + strlen(from) > MAXNAMESIZE) {
		printf ("%s%s\n", to, from);
		error ("Name is too long");
	}
	return strcat(to, from);
}


/*********************************************************************/
/* hash access */
int getHashKey(const char *name)
{
	int k = 0;
	int i;

	for (i = strlen(name) -1; i >= 0; i--)
		k = (k * 137)+name[i];
	//printf("%s , %d hashkey = %d\n",name, k, k % MAXVARIABLES); //for DEBUG
	return (k > 0 ? k : -k) % MAXVARIABLES;
}

varrec *getvariable(const char *name, int purpose)
{
	static int hashcnt = MAXVARIABLES - 1;
	int i = 0;
	varrec *v, *pv;

	// seek
	i = getHashKey(name);
	v = &(v_area[i]);

	for (pv = NULL; v != NULL; v = v->next) {
		pv = v;
		if (!strcmp(name, v->name)) {
			// found.
			CHKERR (purpose == HASH_INSERT, "Label already decrared");
			return v;
		}
	}

	// not found
	if (purpose == HASH_SEEK) {
		sprintf(str, "Label or variable is not found: %s", name);
		error(str);
	}

	// new valiable, no conflict.
	if (v_area[i].name[0]==0) {
		//		printf("no conflict %s %d\n", name, i );
		strcpy(v_area[i].name, name);
		return &(v_area[i]);
	}

	// new valiable, conflict.
	while (v_area[hashcnt].name[0]!=0) {
		CHKERR (--hashcnt < 0, "V-area overflow");
	}
	// printf("conflict %s -> next = %s,  %04d\n",pv->name, name, hashcnt );

	pv->next = &(v_area[hashcnt]);
	strcpy(v_area[hashcnt].name, name);
	return &(v_area[hashcnt]);
}


/*********************************************************************/
/* scanner(lexer) */
#define getch() {ch = *pc++;}
//void getch() {ch = *pc++;}

int getsym()
{
	int k = 0;
	str[0] = '\0';

	while (ISSPACE(ch))
		getch();
	if (ch == '#')
		while (ch != '\n' && ch != '\0')
			getch();
	if (ISALPHA(ch)) {
		while (ISALPHA(ch) || ISNUMERIC(ch)) {
			if (('a' <= ch) && (ch <= 'z'))
				ch += ('A' - 'a');
			if (k < MAXNAMESIZE - 1)
				str[k++] = ch;
			getch();
		}
		str[k] = '\0';
		sym = getvariable(str, HASH_GET)->sym;
	} else if (ISNUMERIC(ch)) {
		num = 0;
		sym = NUMSYM;
		while (ISNUMERIC(ch)) {
			num = 10 * num + (ch - '0');
			getch();
		}
#ifdef REAL
		if (ch == '.') {
			k = 10; // [suspend] type of 'k' must be NUMBER?
			getch();
			while (ISNUMERIC(ch)) {
				num = num + (ch - '0') / (NUMBER)k;
				getch();
				k = k * 10;
			}
		}
#endif

	} else if (ch == '"') {
		sym = STRINGSYM;
		getch();
		while (ch != '"') {
			CHKERR (k > MAXSTRSIZE, "String is too long");
			CHKERR (ch == '\n' || ch == '\0',
			        "Missing terminating '\"' character");
			str[k++] = ch;
			getch();
		}
		str[k] = '\0';
		getch();
	} else {
		sym = ch;
		if (ch != '\0')
			getch();
	}
	return sym;
}

void resetsym(char *p)
{
	pc = p;
	ch = ' ';
	getch();
	getsym();
}


/*********************************************************************/
/* expression parser */
varrec *variable()
{
	char s[MAXNAMESIZE+1];
	varrec * v;

	strcpy(s, str);
	getsym();
	if (sym == '(') {
		safecat(s, "(");
		do {
			getsym();
			num = expression(0);
			sprintf(str, "%d", (int)num);
			safecat(s, str);
			if (sym == ',')
				safecat(s, ",");
		} while ( sym == ',');
		CHKERR(sym != ')', "Missing terminating ')' character")
		safecat(s, ")");
		v = getvariable(s, HASH_GET);
		num = v->value;
		getsym();
	} else {
		v = getvariable(s, HASH_GET);
		num = v->value;
	}
	return v;
}

NUMBER factor(int pre)
{
	NUMBER n;

	if (sym == IDENTSYM) {
		variable();
		n = num;
	} else if (sym == NUMSYM) {
		n = num;
		getsym();
	} else if (sym == '(') {
		getsym();
		n = expression(0);
		CHKERR (sym != ')', "Missing terminating ')' character");
		getsym();
	} else {
		error("Syntax error");
	}
	return num = n;
}

NUMBER monadic(int pre)
{
	if (sym == INTSYM)
		return num = (int)factor(getsym());
	else if (sym == '+')
		return num = factor(getsym());
	else if (sym == '-')
		return num = - factor(getsym());
	else
		return num = factor(0);
}

NUMBER muldiv(int pre)
{
	NUMBER n = monadic(0);
	int tmpsym;

	while (sym == '*' || sym == '/' || sym == MODSYM) {
		tmpsym = sym;
		if (sym == '*') {
			n *= monadic(getsym()) ;
		} else {
			CHKERR (monadic(getsym()) == 0,
			        "Can't be divided by 0");
			if (tmpsym == '/')
				n /= num;
			else
				n = (long)n % (long)num; // [suspend] why long?
		}
	}
	return num = n;
}

NUMBER addsub(int pre)
{
	NUMBER n = muldiv( 0 );

	while (sym == '+' || sym == '-') {
		if (sym == '+')
			n += muldiv(getsym());
		else
			n -= muldiv(getsym());
	}
	return num = n;
}

NUMBER compare(int pre)
{
	NUMBER n = addsub(0);

	if (sym == '=') {
		n = (n == addsub(getsym())) ? 1 : 0;
	} else if (sym == '>') {
		getsym();
		if (sym == '=')
			n = (n >= addsub(getsym())) ? 1 : 0;
		else
			n = (n > addsub( 0 )) ? 1 : 0;
	} else if (sym == '<') {
		getsym();
		if (sym == '>')
			n = (n != addsub(getsym())) ? 1 : 0;
		else if (sym == '=')
			n = (n <= addsub(getsym())) ? 1 : 0;
		else
			n = (n < addsub( 0 )) ? 1 : 0;
	}
	return num = n;
}

NUMBER invert(int pre)
{
	if (sym == NOTSYM)
		return num = (compare(getsym()) != 0) ? 0 : 1;
	return num = compare(0);
}

NUMBER andor(int pre)
{
	NUMBER n = invert(0);

	while (sym == ANDSYM || sym == ORSYM) {
		if (sym == ANDSYM)
			n = (invert(getsym()) && n) ? 1 : 0;
		else
			n = (invert(getsym()) || n) ? 1 : 0;
	}
	return num = n;
}


/*********************************************************************/
/* statements parser */
void statement()
{
	NUMBER n = 0;
	varrec *v;
	char s[MAXNAMESIZE + 1];

	switch (sym) {
	case PRINTSYM:
		do {
			getsym();
			if ((sym == ':') || (sym == '\n')|| (sym == '\0'))
				break;
			if (sym == STRINGSYM) {
				printf("%s",str);
				getsym();
			} else {
				expression(0);
				printf(NUMBERFORMAT, num);
			}
			n = sym;
		} while (sym == ';');
		if (n != ';')
			printf("\n");
		break;

	case INPUTSYM:
		getsym();
		if (sym == STRINGSYM) {
			printf("%s",str);
			getsym();
			CHKERR (sym != ';', "Missing ';'");
			getsym();
		}
		CHKERR (sym != IDENTSYM, "missing variable identifier");
		v = variable();
		CHKERR (scanf(INPUTNUMBERFORMAT, &(v->value)) == 0,
		        "Number input error");
		break;

	case IFSYM:
		n = expression(getsym());
		CHKERR ( sym != THENSYM, "Missing 'THEN'");
		getsym();
		if (n == 0) {
			while (sym != '\n' && sym != '\0')
				getsym();
		} else {
			statement();
		}
		break;

	case GOTOSYM:
		getsym();
		CHKERR (sym != IDENTSYM, "Missing label idenrifier");
		safecat(str, ":");
		v = getvariable(str, HASH_SEEK);
		resetsym(v->ptr);
		break;

	case GOSUBSYM:
		sprintf(s,"GS:%d",++gsptr);
		v = getvariable(s, HASH_GET);
		getsym();
		v->ptr = pc -1;
		CHKERR (sym != IDENTSYM, "Missing label idenrifier");
		safecat(str, ":");
		v = getvariable(str, HASH_SEEK);
		resetsym(v->ptr);
		break;

	case RETURNSYM:
		CHKERR (gsptr <= 0, "'RETURN' without 'GOSUB'");
		sprintf(s,"GS:%d",gsptr--);
		v = getvariable(s, HASH_SEEK);
		resetsym(v->ptr);
		break;

	case FORSYM:
		sprintf(s,"FS:%d",++fsptr);
		v = getvariable(s, HASH_GET);
		v->ptr = pc -1;
		getsym(); // variable

		CHKERR (sym != IDENTSYM, "Missing variable identifier");
		v = variable();
		CHKERR (sym != '=', "Missing '='");
		expression(getsym());
		v->value = num;
		CHKERR ( sym != TOSYM, "Missing 'TO'");
		expression(getsym());
		if ( sym == STEPSYM)
			expression(getsym());
		break;

	case NEXTSYM: {
			char *p = pc -1;

			NUMBER startingNumber;
			int increaseFlag = 1;
			varrec *v2;

			getsym();
			v = NULL;
			if (sym == IDENTSYM)
				v = variable();
			CHKERR (fsptr <= 0, "'NEXT' without 'FOR'");
			sprintf(s,"FS:%d",fsptr);
			v2 = getvariable(s, HASH_SEEK);
			resetsym(v2->ptr);

			v2 = variable();
			if (v != NULL && strcmp(v->name, v2->name)) {
				resetsym(p);
				error ("Mismatch variable");
			}
			expression(getsym());
			startingNumber = num;
			n = expression(getsym());
			if (startingNumber > num)
				increaseFlag = 0;

			if ( sym == STEPSYM) {
				expression(getsym());
				v2->value += num;
			} else {
				v2->value++;
			}
			if ( (increaseFlag && (v2->value > n))
			        || (!increaseFlag && (v2->value < n))) {
				resetsym(p);
				if (sym == IDENTSYM) // skip a variable after 'NEXT'
					v = variable();
				fsptr--;
			}
		}
		break;

	case LABELSYM:
		getsym();
		getsym();
		break;

	case ENDSYM:
		sym = '\0';
		break;

	case IDENTSYM:
		v = variable();
		CHKERR (sym != '=', "Missing '='");
		expression(getsym());
		v->value = num;
		break;

	default:
		break;
	}
}

void statements()
{
	statement();
	while (sym != '\0') {
		CHKERR (sym != ':' && sym != '\n',
		        "Missing ':' or line-feed");
		getsym();
		statement();
	}
}


/*********************************************************************/
/* interpreter */
void readlabel()
{
	varrec *v;

	while (sym != '\0') {
		if (!strcmp(str, "LABEL")) {
			getsym();
			CHKERR (sym != IDENTSYM, "Missing label identifier");
			safecat(str, ":");
			v = getvariable(str, HASH_INSERT);
			v->ptr = pc -1;
		}
		getsym();
	}
}

void interpreter(char *source)
{
	int i;

	for (i = 0; i < MAXVARIABLES; i++) {
		v_area[i].name[0] = '\0';
		v_area[i].value = 0;
		v_area[i].sym = IDENTSYM;
		v_area[i].next = NULL;
	}

	RESERVE("PRINT", PRINTSYM);
	RESERVE("INPUT", INPUTSYM);
	RESERVE("IF",    IFSYM);
	RESERVE("THEN",  THENSYM);
	RESERVE("LABEL", LABELSYM);
	RESERVE("GOTO",  GOTOSYM);
	RESERVE("GOSUB", GOSUBSYM);
	RESERVE("RETURN", RETURNSYM);
	RESERVE("END",   ENDSYM);
	RESERVE("AND",   ANDSYM);
	RESERVE("OR",    ORSYM);
	RESERVE("NOT",   NOTSYM);
	RESERVE("MOD",   MODSYM);
	RESERVE("FOR",   FORSYM);
	RESERVE("TO",    TOSYM);
	RESERVE("NEXT",  NEXTSYM);
	RESERVE("STEP",  STEPSYM);
	RESERVE("INT",   INTSYM);

	orgpc = source;
	resetsym(source);
	readlabel();

	gsptr = fsptr = 0;
	resetsym(source);
	statements();
	printf("ok.\n");
}


/*********************************************************************/
/* main */
int main(int argc, char *argv[])
{
	FILE *sourcefile;
	static char source[MAXSOURCESIZE + 1];
	int i;

	if (argc < 2) {
		printf("usage: cb [filename]\n1-line mode is now available.\n");
//		gets(source); // [suspend] cause warning message by gcc for security problem.
		fgets(source, MAXSOURCESIZE + 1, stdin);
	} else {
		sourcefile = fopen(argv[1], "r");
		if (sourcefile == NULL) {
			printf("cannot open file.\n");
			exit(1);
		}
		i = fread(source, 1, MAXSOURCESIZE , sourcefile);
		fclose(sourcefile);
		source[i] = '\0';
	}

	interpreter(source);
	exit(0);
}
