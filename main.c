#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL) error("not enough memory");
#define SAFEALLOCN(var,Type,n) if((var=(Type*)malloc(sizeof(Type)*(n)))==NULL) error("not enough memory");

typedef struct Token {
    int code; // codul (numele)
    union {
        char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
        long int i; // folosit pentru CT_INT, CT_CHAR
        double r; // folosit pentru CT_REAL
    };
    int line; // linia din fisierul de intrare
    struct Token *next; // inlantuire la urmatorul AL
}Token;

char inbuf[30001];
char *pCrtCh;            // pointer la char
int line=1;            // linia curenta
Token *tokens;        // inceputul listei de atomi
Token *lastToken;        // ultimul atom din lista

enum{ID, CT_INT, CT_REAL, CT_CHAR, CT_STRING, COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, RBRACKET,
    LACC, RACC, ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ,
    GREATER, GREATEREQ, END, SPACE, LINECOMMENT, COMMENT,
    BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE
};

void error(const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error: ");
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk,const char *fmt,...) {
    va_list va;
    va_start(va,fmt);
    fprintf(stderr,"error in line %d: ",tk->line);
    vfprintf(stderr,fmt,va);
    fputc('\n',stderr);
    va_end(va);
    exit(-1);
}

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk,Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if(lastToken){
        lastToken->next = tk;
    }else{
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}
char* createString(const char *start, const char *last) {
    int  n=(last-start);
    char *p;
    SAFEALLOCN(p,char,n+1)
    memcpy(p,start,n);
    p[n]='\0';
    return p;
}

//abfnrtv\'?\"\\0
int escapeSequences(char c) {
    switch (c) {
            case 'a':  return '\a';  break;
            case 'b':  return '\b';  break;
            case 'e':  return '\e';  break;
            case 'f':  return '\f';  break;
            case 'n':  return '\n';  break;
            case 'r':  return '\r';  break;
            case 't':  return '\t';  break;
            case 'v':  return '\v';  break;
            case '\\': return '\\';  break;
            case '\'': return '\'';  break;
            case '\"': return '\"';  break;
            case '?':  return '\?';  break;
            case '0':  return '\0';  break;
            default:
                return -1;

    }
}

char *stringEscapeSequences(char *begin, char *after){
    int i = 0,k=0;
    long n=after-begin;
    char *p=(char* )malloc(n*sizeof(char));
    if(begin[0]=='\"'){
        for(i=1;i<n-1;i++){
            if(begin[i]=='\\')
            {
                if(escapeSequences(begin[i+1])>=0)
                {
                    p[k++]=escapeSequences(begin[i+1]);
                    i++;
                }

            }
            else p[k++]=begin[i];
        }
    }

    p[i]='\0';
    return p;
}
int flag = 0;

int getNextTk() {

    int state = 0;
    long nCh;
    char ch;
    const char *pStartCh = pCrtCh;
    char *s;
    Token *tk;
    while(1){
        ch = *pCrtCh;
        if(ch == EOF)
            return END;
        switch(state){
            case 0:
            {

                if(isalpha(ch)){
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 1;

                }

                else if(ch == ' ' || ch == '\r' || ch == '\t'){
                    pCrtCh++;

                }
                else if(ch == '\n'){
                    line++;
                    pCrtCh++;

                }
                else if(ch == 0){ // sfarsit de sir
                    addTk(END);
                    return END;
                }
                else if(ch == '0') {
                    pCrtCh++;
                    state = 10;

                }
                else if(ch > '0' && ch <= '9') {
                    pCrtCh++;
                    state = 3;

                }
                else if(ch == '\'')
                {
                    pStartCh = ++pCrtCh;
                    state = 15;

                }
                else if(ch == '\"')
                {
                    pCrtCh++;
                    state = 19;

                }
                else if(ch == ',')
                {
                    pCrtCh++;
                    state = 25;

                }
                else if(ch == ';')
                {
                    pCrtCh++;
                    state = 26;

                }
                else if(ch == '(')
                {
                    pCrtCh++;
                    state = 27;

                }
                else if(ch == ')')
                {
                    pCrtCh++;
                    state = 28;

                }
                else if(ch == '[')
                {
                    pCrtCh++;
                    state = 29;

                }
                else if(ch == ']')
                {
                    pCrtCh++;
                    state = 30;

                }
                else if(ch == '{')
                {
                    pCrtCh++;
                    state = 31;

                }
                else if(ch == '}')
                {
                    pCrtCh++;
                    state = 32;

                }
                else if(ch == '+')
                {
                    pCrtCh++;
                    state = 33;

                }
                else if(ch == '-')
                {
                    pCrtCh++;
                    state = 34;

                }
                else if(ch == '*')
                {
                    pCrtCh++;
                    state = 35;

                }
                else if(ch == '/')
                {
                    pCrtCh++;
                    state = 36;

                }
                else if(ch == '.')
                {
                    pCrtCh++;
                    state = 41;

                }
                else if(ch == '&')
                {
                    pCrtCh++;
                    state = 42;

                }
                else if(ch == '|')
                {
                    pCrtCh++;
                    state = 44;

                }
                else if(ch == '!')
                {
                    pCrtCh++;
                    state = 46;

                }
                else if(ch == '=')
                {
                    pCrtCh++;
                    state = 49;

                }
                else if(ch == '<')
                {
                    pCrtCh++;
                    state = 52;

                }
                else if(ch == '>')
                {
                    pCrtCh++;
                    state = 55;

                }
                else if(ch == '\0' || ch == EOF)
                {
                    pCrtCh++;
                    state = 58;

                }
                else
                    tkerr(addTk(END),"caracter invalid");
                break;
            }
            case 1:
			{
                if(isalnum(ch) || ch == '_')
                    pCrtCh++;
                else
                    state = 2;
                break;
			}
			case 2:
            {
                //nCh = pCrtCh - pStartCh + 1; // lungimea cuvantului gasit
                nCh = pCrtCh - pStartCh; // lungimea cuvantului gasit
                // teste cuvinte cheie
                if(nCh == 5 && !memcmp(pStartCh,"break",5))
                    tk = addTk(BREAK);
                else if(nCh == 4 && !memcmp(pStartCh,"char",4))
                    tk = addTk(CHAR);
                else if(nCh == 6 && !memcmp(pStartCh,"double",6))
                    tk = addTk(DOUBLE);
                else if(nCh == 4 && !memcmp(pStartCh,"else",4))
                    tk = addTk(ELSE);
                else if(nCh == 3 && !memcmp(pStartCh,"for",3))
                    tk = addTk(FOR);
                else if(nCh == 2 && !memcmp(pStartCh,"if",2))
                    tk = addTk(IF);
                else if(nCh == 3 && !memcmp(pStartCh,"int",3))
                    tk = addTk(INT);
                else if(nCh == 6 && !memcmp(pStartCh,"return",6))
                    tk = addTk(RETURN);
                else if(nCh == 6 && !memcmp(pStartCh,"struct",6))
                    tk = addTk(STRUCT);
                else if(nCh == 4 && !memcmp(pStartCh,"void",4))
                    tk = addTk(VOID);
                else if(nCh == 5 && !memcmp(pStartCh,"while",5))
                    tk = addTk(WHILE);
                // � toate cuvintele cheie �
                else{ // daca nu este un cuvant cheie, atunci e un ID
                    tk = addTk(ID);
                    tk->text = createString(pStartCh,pCrtCh);
                }
                return tk->code;
            }
			case 3:
			{
				if(isdigit(ch))
                {
                    pCrtCh++;
                }
				else if(ch == '.')
				{
					pCrtCh++;
					state = 4;
				}
				else if(ch=='e' || ch=='E')
				{
					pCrtCh++;
					state = 7;
				}
				else
                {
                    state = 12;
                }
				break;
			}
			case 4:
            {
                if(isdigit(ch))
                {
                    pCrtCh++;
                    state = 5;
                }
                break;
            }
            case 5:
            {
                if(isdigit(ch))
                {
                    pCrtCh++;
                    state = 5;
                }
                else if(ch == 'e' || ch == 'E')
                {
                    state = 7;
                    pCrtCh++;
                }
                else
                {
                    state = 6;
                }
                break;
            }
            case 6:
            {
                tk = addTk(CT_REAL);
                s = createString(pStartCh, pCrtCh);
                tk -> r = strtof(s, &s);
                return CT_REAL;
            }
            case 7:
            {
                if(ch == '+' || ch == '-')
                {
                    pCrtCh++;
                    state = 8;
                }
                else if(isdigit(ch))
                {
                    pCrtCh++;
                    state = 9;
                }
                break;
            }
            case 8:
            {
                if(isdigit(ch))
                {
                    pCrtCh++;
                    state = 9;
                }
                break;
            }
            case 9:
            {
                if(isdigit(ch))
                {
                    pCrtCh++;
                    state = 9;
                }
                else
                {
                    state = 6;
                }
                break;
            }
            case 10:
            {
                if(ch == 'x')
                {
                    pCrtCh++;
                    state = 11;
                }
                else if(ch >= '0' && ch<='7')
                {
                    pCrtCh++;
                    state = 13;
                }
                else if(ch >= '8' && ch<='9')
                {
                    pCrtCh++;
                    state = 14;
                }
                else if(ch == '.')
                {
                    pCrtCh++;
                    state = 4;
                }
                else
                {
                    state = 12;
                }
                break;
            }
            case 11:
            {
                if(isalnum(ch))
                {
                   pCrtCh++;
                   state = 11;
                }
                else
                {
                    state = 12;
                }
                break;
            }
            case 12:
            {
                tk = addTk(CT_INT);
                s = createString(pStartCh, pCrtCh);
                if((strchr(s, 'x') != 0) || (strchr(s, 'X') != 0)) {
                    tk -> i = strtol(s, NULL, 16); // base 16
                }
                else if(*s == '0') {
                    tk -> i = strtol(s, NULL, 8); // base 8
                }
                else {
                    tk -> i = strtol(s, NULL, 10); // base 10
                }
                return CT_INT;
            }
            case 13:
            {
                if(ch >= '0' && ch<='7')
                {
                    pCrtCh++;
                    state = 13;
                }
                else if(ch >= '8' && ch<='9')
                {
                    pCrtCh++;
                    state = 14;
                }
                else
                {
                    state = 12;
                }
                break;
            }
            case 14:
            {
                if(ch >= '8' && ch<='9')
                {
                    pCrtCh++;
                    state = 14;
                }
                else if(ch == '.')
                {
                    pCrtCh++;
                    state = 4;
                }
                else if(ch == 'e' || ch == 'E')
                {
                    pCrtCh++;
                    state = 7;
                }
                break;
            }
            case 15:
            {
                if(ch == '\\')
                {
                    pCrtCh++;
                    state = 16;
                }
                else
                {
                    pCrtCh++;
                    state = 17;
                }
                break;
            }
            case 16:
            {
                if(ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't' || ch == 'v' || ch == '\'' || ch == '\"' || ch == '\\' || ch == '0')
                {
                    pCrtCh++;
                    state = 17;
                }
                break;
            }
            case 17:
            {
                if(ch == '\'')
                {
                    pCrtCh++;
                    state = 18;
                }
                break;
            }
            case 18:
            {
                tk = addTk(CT_CHAR);
                s = createString(pStartCh, pCrtCh);
                tk -> i = *s;
                return CT_CHAR;
            }
            case 19:
            {
                if(ch == '\\') {
                    pCrtCh++;
                    state = 20;
                }
                else if(ch == '\"')
                {
                    pCrtCh++;
                    state = 24; //CT_STRING
                }
                else
                {
                    pCrtCh++;
                    state = 22;
                }
                break;
            }
            case 20:
            {
                if(strchr("abfrtv\'?\"\\\0", ch))
                {
                    pCrtCh++;
                    state = 21;
                }
                else if(ch == 'n')
                {
                    pCrtCh++;
                    state = 21;
                    line++;
                }
                break;
            }
            case 21:
            {
                if(ch == '\"')
                {
                    pCrtCh++;
                    state = 24; //CT_STRING
                }
                else
                {
                    pCrtCh++;
                    state = 19;
                }
                break;
            }
            case 22:
            {
              if(ch == '\\')
              {
                  pCrtCh++;
                  state = 20;
              }
              else if (ch == '\"')
              {
                  pCrtCh++;
                  state = 24; //CT_String
              }
              else
              {
                  pCrtCh++;
                  state = 22;
              }
              break;
            }
            case 24:
            {
                tk = addTk(CT_STRING);
                tk -> text = stringEscapeSequences(pStartCh, pCrtCh);
                return CT_STRING;
            }
            case 25:
                addTk(COMMA);
                return COMMA;
            case 26:
                addTk(SEMICOLON);
                return SEMICOLON;
            case 27:
                addTk(LPAR);
                return LPAR;
            case 28:
                addTk(RPAR);
                return RPAR;
            case 29:
                addTk(LBRACKET);
                return LBRACKET;
            case 30:
                addTk(RBRACKET);
                return RBRACKET;
            case 31:
                addTk(LACC);
                return LACC;
            case 32:
                addTk(RACC);
                return RACC;
            case 33:
                addTk(ADD);
                return ADD;
            case 34:
                addTk(SUB);
                return SUB;
            case 35:
                addTk(MUL);
                return MUL;
            case 36:
            {
                if(ch == '/')
                {
                    state = 37;
                    pCrtCh++;
                }
                else if(ch == '*')
                {
                    state = 38;
                    pCrtCh++;
                }
                else
                {
                    state = 40; //DIV
                    //pCrtCh++;
                }
                break;
            }
            case 37:
            {
                if(ch == '\n')
                {
                   state = 0;
                   pCrtCh++;
                   line++;
                }
                else if(!strchr("\n\r\0",ch))
                {
                    state = 37;
                    pCrtCh++;
                }
                else
                {
                    state = 0;
                    pCrtCh++;
                }
                break;
            }
            case 38:
            {
                if(ch == '\n')
                {
                    state = 38;
                    line++;
                    pCrtCh++;
                }
                else if(ch != '*')
                {
                    state = 38;
                    pCrtCh++;
                }
                else
                {
                    state = 39;
                    pCrtCh++;
                }
                break;
            }
            case 39:
            {
                if(!strchr("*/",ch))
                {
                   state = 38;
                   pCrtCh++;
                }
                else if(ch == '*')
                {
                    state = 39;
                    pCrtCh;
                }
                else if(ch == '/')
                {
                    state = 0;
                    pCrtCh++;
                }
                break;
            }
            case 40:
                addTk(DIV);
                return DIV;
            case 41:
                addTk(DOT);
                return DOT;
            case 42:
                if(ch == '&') {
                    state = 43;
                    pCrtCh++;
                }
                break;
            case 43:
                addTk(AND);
                return AND;
            case 44:
                if(ch == '|')
                    state = 45;
                break;
            case 45:
                addTk(OR);
                return OR;
            case 46:
                if(ch == '=') {
                    pCrtCh++;
                    state = 48;
                }
                else
                    state = 47;
                break;
            case 47:
                addTk(NOT);
                return NOT;
            case 48:
                addTk(NOTEQ);
                return NOTEQ;
            case 49:
                if(ch == '=') {
                    pCrtCh++;
                    state = 51;
                }
                else
                    state = 50;
                break;
            case 50:
                addTk(ASSIGN);
                return ASSIGN;
            case 51:
                addTk(EQUAL);
                return EQUAL;
            case 52:
                if(ch == '=') {
                    pCrtCh++;
                    state = 54;
                }
                else
                    state = 53;
                break;
            case 53:
                addTk(LESS);
                return LESS;
            case 54:
                addTk(LESSEQ);
                return LESSEQ;
            case 55:
                {
                  if(ch == '=') {
                    pCrtCh++;
                    state = 57;
                }
                else
                    state = 56;
                break;
                }

            case 56:
                addTk(GREATER);
                return GREATER;
            case 57:
                addTk(GREATEREQ);
                return GREATEREQ;
            case 58:
                addTk(END);
                return END;

        }
    }
    return 0;
}

    Token *crtTk;

int unit()
{
    Token *startTk=crtTk;
    for(;;)
    {
        if(!declStruct() && !declFunc() && !declVar())
        {
            break;
        }
    }
    if(consume(END))
    {
        return 1;
    }
    crtTk=startTk;
    return 0;
}

int consume(int code)//Consuma atomi terminali
{
    lastToken=crtTk;
    if(crtTk->code==code)
    {
        crtTk=crtTk->next;
        return 1;
    }
    else
    {
        return 0;
    }
}

int funcArg()
{
    Token *startTk=crtTk;
    if(typeBase())
    {
        if(consume(ID))
        {
            arrayDecl();
            return 1;
        }
        else
        {
            tkerr(crtTk,"Missing ID after typeBase!");
        }

    }
    startTk=crtTk;
    return 0;
}

int typeBase()
{
    Token *startTk=crtTk;
    if(consume(INT) || consume(DOUBLE) || consume(CHAR))
        return 1;
    else
        if(consume(STRUCT))
        {
            if(consume(ID))
                return 1;
            else
            {
                tkerr(crtTk,"Missing ID for STRUCT");
            }

        }
    crtTk=startTk;
    return 0;

}

int arrayDecl()
{
    Token *startTk=crtTk;
    if(consume(LBRACKET))
    {
        expr();
        if(consume(RBRACKET))
            return 1;
        else
        {
            tkerr(crtTk,"Missing right bracket!");
        }
    }
    crtTk=startTk;
    return 0;
}


int declVar()
{
    Token *startTk=crtTk;
    if(typeBase())
    {
        if(consume(ID))
        {
            arrayDecl();
            for(;;)
            {
                if(consume(COMMA))
                {
                    if(consume(ID))
                    {
                        arrayDecl();
                    }
                    else
                    {
                        tkerr(crtTk,"Missing ID after comma!");
                    }

                }
                else
                break;
            }
            if(consume(SEMICOLON))
            {
                return 1;
            }
            else
            {
                tkerr(crtTk,"Missing semicolon!");
            }
        }
        else
        {
            tkerr(crtTk,"Missing ID after typeBase!");
        }

    }
    crtTk=startTk;
    return 0;
}

int declStruct()
{
    Token *startTk=crtTk;
    if(consume(STRUCT))
    {
        if(consume(ID))
        {
            if(consume(LACC))
            {
                for(;;)
                {
                    if(!declVar())
                    break;
                }
                if(consume(RACC))
                {
                    if(consume(SEMICOLON))
                        return 1;
                    else
                    {
                        tkerr(crtTk,"Missing semicolon!");
                    }
                }
                else tkerr(crtTk,"Missing right accolade!");
            }
        }
        else
        {
            tkerr(crtTk,"Missing ID for STRUCT");
        }

    }
    crtTk=startTk;
    return 0;
}

int typeName()
{
    Token *startTk=crtTk;
    if(typeBase())
    {
        arrayDecl();
        return 1;
    }
    crtTk=startTk;
    return 0;
}

int declFunc()
{
    Token *startTk=crtTk;
    if(typeBase())
    {
        consume(MUL);
        if(consume(ID))
        {
            if(consume(LPAR))
            {
                funcArg();
                for(;;)
                {
                    if(consume(COMMA))
                    {
                        if(!funcArg())
                        {
                            tkerr(crtTk,"Missing function argumnets after COMMA!");
                            break;
                        }
                    }
                    else
                        {
                            break;
                        }

                }
                if(consume(RPAR))
                {
                    if(stmCompound())
                    {
                        return 1;
                    }
                    else tkerr(crtTk,"Missing compound for function!");
                }
                else tkerr(crtTk,"Missing RPAR in function!");

            }
        }
        else
        {
            tkerr(crtTk,"Missing ID after typeBase!");
        }
    }
    if(consume(VOID))
    {
        if(consume(ID))
        {
            if(consume(LPAR))
            {
                if(funcArg())
                {
                    for(;;)
                    {
                        if(consume(COMMA))
                            if(!funcArg())
                            {
                                tkerr(crtTk,"Missing function argumnets after COMMA!");
                            }
                            else
                            break;
                    }
                    if(consume(RPAR))
                    {
                        if(stmCompound())
                        {
                            return 1;
                        }
                        else tkerr(crtTk,"Missing compound for function!");
                    }
                        else tkerr(crtTk,"Missing RPAR in function!");
                }
                if(consume(RPAR))
                    {
                        if(stmCompound())
                        {
                            return 1;
                        }
                        else tkerr(crtTk,"Missing compound for function!");
                    }
                    else tkerr(crtTk,"Missing RPAR in function!");

            }
            else
            {
                tkerr(crtTk,"Missing LPAR after ID in function!");
            }

        }
        else
        {
            tkerr(crtTk,"Missing ID for function!");
        }
    }
    crtTk=startTk;
    return 0;
}

int stm()
{
    Token *startTk=crtTk;
    if(stmCompound())
    {
        return 1;
    }
    if(consume(IF))
    {
        if(consume(LPAR))
        {
            if(expr())
            {
                if(consume(RPAR))
                {
                    if(stm())
                    {
                        if(consume(ELSE))
                        {
                            if(stm())
                                return 1;
                            else tkerr(crtTk,"Missing stm after ELSE!");
                        }
                        return 1;
                    }
                    else tkerr(crtTk,"Missing stm after RPAR!");
                }
                else tkerr(crtTk,"Missing RPAR after expr!");
            }
            else tkerr(crtTk,"Missing expr after LPAR!");
        }
        else tkerr(crtTk,"Missing LPAR after IF!");
    }
    else if(consume(WHILE))
        {
            if(consume(LPAR))
            {
                if(expr())
                {
                    if(consume(RPAR))
                    {
                        if(stm())
                        {
                            return 1;
                        }
                        else tkerr(crtTk,"Missing stm after RPAR!");
                    }
                    else tkerr(crtTk,"Missing RPAR after expr!");
                }
                else tkerr(crtTk,"Missing expr after LPAR!");
            }
            else tkerr(crtTk,"Missing LPAR after WHILE!");
        }
        else
        if(consume(FOR))
        {
            if(consume(LPAR))
            {
                expr();
                if(consume(SEMICOLON))
                {
                    expr();
                    if(consume(SEMICOLON))
                    {
                        expr();
                        if(consume(RPAR))
                        {
                            if(stm())
                            {
                                return 1;
                            }else tkerr(crtTk,"Missing stm after RPAR!");
                        }else tkerr(crtTk,"Missing RPAR!");
                    }else tkerr(crtTk,"Missing second SEMICOLON after LPAR!");
                }
                else tkerr(crtTk,"Missing SEMICOLON after LPAR!");
            }
            else tkerr(crtTk,"Missing LPAR after FOR!");
        }
        else
        if(consume(BREAK))
        {
            if(consume(SEMICOLON))
            {
                return 1;
            }
            else tkerr(crtTk,"Missing SEMICOLON after BREAK!");

        }
        else
        if(consume(RETURN))
        {
            expr();
            if(consume(SEMICOLON))
            {
                return 1;
            }
            else tkerr(crtTk,"Missing SEMICOLON after RETURN!");
        }
        else
        {
            if(expr())
            {
                if(consume(SEMICOLON))
                {
                    return 1;
                }
                else tkerr(crtTk,"Missing SEMICOLON after expr!");
            }
            else if(consume(SEMICOLON))
            {
                return 1;
            }
        }
        crtTk=startTk;
        return 0;
}

int stmCompound()
{
    Token *startTk=crtTk;
    if(consume(LACC))
    {
        for(;;)
        {
            if(!declVar() && !stm())
            {
                break;
            }
        }
        if(consume(RACC))
        {
            return 1;
        }
        else
        {
            tkerr(crtTk,"Missing RACC!");
        }

    }
    crtTk=startTk;
    return 0;
}

int expr()
{
    Token *startTk=crtTk;
    if(exprAssign())
    {
        return 1;
    }
    crtTk=startTk;
    return 0;
}

int exprAssign()
{
    Token *startTk=crtTk;
    if(exprUnary())
    {
        if(consume(ASSIGN))
        {
            if(exprAssign())
            {
                return 1;
            }
            else
            {
                tkerr(crtTk,"Missing an expression after ASSIGN!");
            }
        }
    }
    crtTk=startTk;
    if(exprOr())
    {
        return 1;
    }
    crtTk=startTk;
    return 0;
}

int exprOrPrim()
{
    if(consume(OR))
    {
        if(exprAnd())
        {
            if(exprOrPrim())
            {;
                return 1;
            }
        }
        else
        {
            tkerr(crtTk,"Missing exprAnd after OR!");
        }

    }
    return 1;
}
int exprOr()
{
    Token *startTk=crtTk;
    if(exprAnd())
    {
        if(exprOrPrim())
        {
            return 1;
        }
        else tkerr(crtTk,"Missing and expression after exprAnd()");
    }
    crtTk=startTk;
    return 0;
}

int exprAndPrim()
{
    if(consume(AND))
    {
        if(exprEq())
        {
            if(exprAndPrim())
            {
                return 1;
            }
        }
        else
        {
            tkerr(crtTk,"Missing expression after AND!");
        }
    }
    return 1;
}
int exprAnd()
{
    Token *startTk=crtTk;
    if(exprEq())
    {
        if(exprAndPrim())
        {
            return 1;
        }
        else tkerr(crtTk,"Missing expression after exprEq!");
    }
    crtTk=startTk;
    return 0;
}

int exprEqPrim()
{
    if(consume(EQUAL) || consume(NOTEQ))
    {
        if(exprRel())
        {
            if(exprEqPrim())
            {
                return 1;
            }
        }
        else
        {
            tkerr(crtTk,"Missing expression after EQUAL or NOTEQ!");
        }

    }
    return 1;
}
int exprEq()
{
    Token *startTk=crtTk;
    if(exprRel())
    {
        if(exprEqPrim())
        {
            return 1;
        }
        else tkerr(crtTk,"Missing expression after exprRel!");
    }
    crtTk=startTk;
    return 0;
}

int exprRelPrim()
{
    if(consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ))
    {
        if(exprAdd())
        {
            if(exprRelPrim())
            {
                return 1;
            }
        }
        else
        {
            tkerr(crtTk,"Missing expression after LESS or LESSEQ or GREATER or GREATEREQ!");
        }
    }
    return 1;
}
int exprRel()
{
    Token *startTk=crtTk;
    if(exprAdd())
    {
        if(exprRelPrim())
        {
            return 1;
        }
        else tkerr(crtTk,"Missing expression after exprAdd");
    }
    crtTk=startTk;
    return 0;
}

int exprAddPrim()
{
    if(consume(ADD) || consume(SUB))
    {
        if(exprMul())
        {
            if(exprAddPrim())
            {
                return 1;
            }
        }
        else
        {
            tkerr(crtTk,"Missing expression after ADD or SUB");
        }
    }
    return 1;
}
int exprAdd()
{
    Token *startTk=crtTk;
    if(exprMul())
    {
        if(exprAddPrim())
        {
            return 1;
        }
        else tkerr(crtTk,"Missing expression after exprMul!");
    }
    crtTk=startTk;
    return 0;
}


int exprMulPrim()
{
    if(consume(MUL) || consume(DIV))
    {
        if(exprCast())
        {
            if(exprMulPrim())
            {
                return 1;
            }
        }
        else
        {
            tkerr(crtTk,"Missing expression after MUL or DIV");
        }
    }
    return 1;
}
int exprMul()
{
    Token *startTk=crtTk;
    if(exprCast())
    {
        if(exprMulPrim())
        {
            return 1;
        }
        else tkerr(crtTk,"Missing expression after exprCast");
    }
    startTk=crtTk;
    return 0;
}


int exprCast()
{
    Token *startTk=crtTk;
    if(consume(LPAR))
    {
        if(typeName())
        {
            if(consume(RPAR))
            {
                if(exprCast())
                {
                    return 1;
                }
            }
            else
                {
                    tkerr(crtTk,"Missing RPAR in cast!");
                }
        }
        else tkerr(crtTk,"Missing typename after LPAR");
    }
    else if(exprUnary())
    {
        return 1;
    }
    crtTk=startTk;
    return 0;
}


int exprUnary()
{
    Token *startTk=crtTk;
    if(consume(SUB) || consume(NOT))
    {
        if(exprUnary())
        {
            return 1;
        }
        else
        {
            tkerr(crtTk,"Missing expression after SUB or NOT!");
        }
    }
    else if(exprPostfix())
    {
        return 1;
    }
    crtTk=startTk;
    return 0;
}

int exprPostfixPrime()
{
    if(consume(LBRACKET))
    {
        if(expr())
        {
            if(consume(RBRACKET))
            {
                if(exprPostfixPrime())
                {
                    return 1;
                }
            }
            else
            {
                tkerr(crtTk,"Missing RBRACKET in postfix!");
            }
        }else tkerr(crtTk,"Missing expression after LBRACKET");
    }
    else
    {
        if(consume(DOT))
        {
            if(consume(ID))
            {
                if(exprPostfixPrime())
                {
                    return 1;
                }
            }
            else
            {
                tkerr(crtTk,"Missing ID after DOT in postfix!");
            }
        }
    }

    return 1;
}

int exprPostfix()
{
    Token *startTk=crtTk;
    if(exprPrimary())
    {
        if(exprPostfixPrime())
        {
            return 1;
        }else tkerr(crtTk,"Missing expression after exprPrimary!");
    }
    crtTk=startTk;
    return 0;
}



int exprPrimary()
{
    Token *startTk=crtTk;
    if(consume(ID))
    {
        if(consume(LPAR))
        {
            if(expr())
            {
                for(;;)
                {
                    if(consume(COMMA))
                    {
                        if(!expr())
                            tkerr(crtTk,"Missing expr after COMMA!");
                    }
                    else
                        break;
                }
            }
            if(consume(RPAR))
               return 1;
            else
            {
                tkerr(crtTk,"Missing LPAR after expression!");
            }

        }
        return 1;
    }
    if(consume(CT_INT))
    {
        return 1;
    }
    if(consume(CT_REAL))
    {
        return 1;
    }
    if(consume(CT_CHAR))
    {
        return 1;
    }
    if(consume(CT_STRING))
    {
        return 1;
    }
    if(consume(LPAR))
    {
        if(expr())
        {
            if(consume(RPAR))
            {
                return 1;
            }
            else
            {
                tkerr(crtTk,"Missing RPAR in expression!");
            }
        }
        else
        {
            tkerr(crtTk,"Missing expr after LPAR!");
        }
    }
    crtTk=startTk;
    return 0;
}

void printAtoms(Token *tk) {
    static char *atomsNames[] = {
            "ID", "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING", "COMMA", "SEMICOLON", "LPAR","RPAR", "LBRACKET",
            "RBRACKET","LACC", "RACC", "ADD", "SUB", "MUL", "DIV", "DOT", "AND", "OR", "NOT", "ASSIGN", "EQUAL", "NOTEQ", "LESS",
            "LESSEQ","GREATER", "GREATEREQ", "END", "SPACE", "LINECOMMENT", "COMMENT",
            "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF", "INT", "RETURN", "STRUCT", "VOID", "WHILE"
    };

    const char *s;
    s = atomsNames[tk->code];

    if(strcmp(s, "CT_INT") == 0)
        printf("CT_INT: %ld ", tk -> i);
    else if(strcmp(s, "CT_CHAR") == 0)
        printf("CT_CHAR: %c ", (char)tk -> i);
    else if(strcmp(s, "CT_REAL") == 0)
        printf("CT_REAL: %.02f ", tk -> r);
    else if((strcmp(s, "CT_STRING") == 0))
        printf("CT_STRING: %s ", tk -> text);
    else if(strcmp(s, "ID") == 0)
        printf("ID: %s ", tk -> text);
    else
        printf("%s", s);
}


int main() {

    FILE *fis;
    if((fis = fopen("C:/Users/aleku/Desktop/LFTC/Compilator/tests/9.c","r")) == NULL) {
        printf("eroare deschidere fisier");
        return 1;
    }

    unsigned long n = fread(inbuf,1,30000,fis);
    inbuf[n] = '\0';
    fclose(fis);
    pCrtCh = inbuf;

    // afisarea atomilor
    while(1){
        if(getNextTk() != END)
        {
            printf("%d  ", lastToken->line);
            printAtoms(lastToken);
            printf("\n");
        }
        else
        {
           printf("%d  ", lastToken->line);
           printAtoms(lastToken);
           printf("\n");
           break;
        }

    }

        crtTk=tokens;
        if(unit()){
            printf("Syntax ok \n");
        }else {
            printf("Error \n");
            return 1;
        }


    return 0;

}
