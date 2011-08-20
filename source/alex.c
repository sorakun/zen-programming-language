/*
 * The lexer.
 * See Copyright Notice in azure.h.
*/

#include <string.h>
#include "zen.h"
#include "alex.h"
#include "aerror.h"
#include "autil.h"

/* Check if an id string is a reserved word. */
int clReservedLookup(char* token)
{
	int i;
	for (i=0; i<sizeof(tokens)/sizeof(token_name); i++)
		if (!strcmp(token, tokens[i].str))
			return tokens[i].token;
	return IDENTIFIER;
}

/* Get the name string for a token. */
char* clGetTokenName(int token)
{
	int i;
	for (i=0; i<sizeof(tokens)/sizeof(token_name); i++)
		if (token == tokens[i].token)
			return tokens[i].str;
	return NULL;
}

/* Open lex structure for reading. */
int clLexOpen(LexInfo* li, const char* source, char* buffer)
{
	int i, pos;
	li->buf = buffer;
	li->line_num = 1;
	li->pos = 0;
	strcpy(li->source, source);
	li->line_size = 1024;
	if (!(li->lineerrors=calloc(li->line_size, sizeof(char)))) return 0;
	for (i=0; i<sizeof(tokens)/sizeof(token_name); i++)
	{
		pos = 0;
		li->regtrees[i].reg = regexp(tokens[i].pattern, &pos, 0);
		li->regtrees[i].first = strlen(tokens[i].str)>0?tokens[i].str[0]:0;
	}
	li->token.dbuflen = li->next_token.dbuflen = ZEN_MAX_LITERAL_LENGTH;
	li->token.dbuf = calloc(li->token.dbuflen, sizeof(char));
	li->next_token.dbuf = calloc(li->next_token.dbuflen, sizeof(char));
	if (!li->token.dbuf || !li->next_token.dbuf)
		return 0;
	clStep(li);	/* one step forward to start the pipeline */
	return 1;
}

/* Close up the structure. */
void clLexClose(LexInfo* li)
{
	int i;
	for (i=0; i<sizeof(tokens)/sizeof(token_name); i++)
		freeregnode(li->regtrees[i].reg);
	free(li->token.dbuf);
	free(li->next_token.dbuf);
	free(li->lineerrors);
}

/* Skip spaces and horizontal tabs, and return #skipped. */
int skipsp(LexInfo* li)
{
	int num = 0;
	for (; li->buf[li->pos]==' '||li->buf[li->pos]=='\t'; li->pos++,num++);
	return num;
}

/* Skip line breaks, and return #skipped. */
uint skipcr(LexInfo* li)
{
	uint num = 0, line_size;
	while (li->buf[li->pos]=='\n' || li->buf[li->pos]=='\r')
	{
		num++;
		if (li->buf[li->pos+1] == 10)
			li->pos++;
		li->line_num++;
		if (li->line_num >= li->line_size)	/* expand line errors */
		{
			line_size = li->line_size;
			li->line_size *= 2;
			li->lineerrors = memexp(li->lineerrors, line_size, li->line_size);
		}
		li->pos++;
	}
	return num;
}

/* Translates all the escape characters in a string. */
void escape(LexInfo* li, char* string)
{
	int i,j;
	char scopy[ZEN_MAX_LITERAL_LENGTH] = {0};
	for (i=0,j=0; i<(int)strlen(string); i++,j++)
	{
		if (string[i] != '\\')
			scopy[j] = string[i];
		else
			switch (string[++i])
			{
				case 'f': scopy[j] = '\f'; break;
				case 'n': scopy[j] = '\n'; break;
				case 'r': scopy[j] = '\r'; break;
				case 't': scopy[j] = '\t'; break;
				case '\"': scopy[j] = '\"'; break;
				case '\\': scopy[j] = '\\'; break;
				default: error("Unknown escape character.");
			}
	}
	strcpy(string, scopy);
}

/* Expand a token's buffer when string is too long. */
int alloctoken(_token* tok, int size)
{
	if (!tok) return 0;
	if (!(tok->dbuf=calloc(size, sizeof(char)+1)))
		return 0;
	return 1;
}

/* Free the dynamically allocated buffer if present. */
void freetoken(_token* tok)
{
	if (tok && tok->dbuf && tok->dbuf!=tok->buf)
		free(tok->dbuf);
}

int copytoken(_token* dest, _token* source)
{
	if (!source || !dest) return 0;
	/* max length not exceeded yet */
	if (strlen(source->dbuf) <= ZEN_MAX_TOKEN_LEN)
	{
		dest->token = source->token;
		strcpy(dest->buf, source->dbuf);
		dest->dbuf = dest->buf;
		dest->dbuflen = strlen(dest->dbuf);
	}
	else
	{
		if (!alloctoken(dest, strlen(source->dbuf)))
			return 0;
		dest->token = source->token;
		strcpy(dest->dbuf, source->dbuf);
		dest->dbuflen = strlen(dest->dbuf);
	}
	return 1;
}

/* The "pulsate" function. Return the next token's id. */
int clStep(LexInfo* li)
{
	int i, pos=0, inescape = 0;	/* are we in an escape sequence? */
	li->token.token = li->next_token.token;
	strcpy(li->token.dbuf, li->next_token.dbuf);
	li->token.dbuflen = li->next_token.dbuflen;
l1:	/* skip spaces and CRs */
	while (strchr(" \t\n\r", li->buf[li->pos]) && li->buf[li->pos])
	{
		li->next_token.seencr += skipcr(li);
		skipsp(li);
	}
	if (li->buf[li->pos] == 0)		/* end of file */
		return li->next_token.token = ENDOFFILE;
	if (li->buf[li->pos] == '\"')	/* escaping literals */
	{
		li->pos++;
		while (li->buf[li->pos]!=0 && !(li->buf[li->pos]=='\"'&&!inescape
			&& pos<ZEN_MAX_LITERAL_LENGTH))
		{
			if (li->buf[li->pos] == '\n') li->line_num++;
			inescape = (li->buf[li->pos]=='\\'&&!inescape);
			li->next_token.dbuf[pos++] = li->buf[li->pos++];
		}
		if (pos>=ZEN_MAX_LITERAL_LENGTH)	/* should set an error here */
			return li->next_token.token = ENDOFFILE;
		li->pos++;
		li->next_token.dbuf[pos] = 0;
		li->next_token.dbuflen = pos;
		escape(li, li->next_token.dbuf);
		li->next_token.token = LITERAL;
		return li->token.token;
	}
	if (li->buf[li->pos]=='\'')		/* non-escaping literals */
	{
		li->pos++;
		while (li->buf[li->pos]!=0 && li->buf[li->pos]!='\''
			&& pos<ZEN_MAX_LITERAL_LENGTH)
		{
			if (li->buf[li->pos] == '\n') li->line_num++;
			li->next_token.dbuf[pos++] = li->buf[li->pos++];
		}
		if (pos>=ZEN_MAX_LITERAL_LENGTH)	/* should set an error here */
			return li->next_token.token = ENDOFFILE;
		li->pos++;
		li->next_token.dbuf[pos] = 0;
		li->next_token.dbuflen = pos;
		li->next_token.token = LITERAL;
		return li->token.token;
	}
	/* single-line comments */
	if (li->buf[li->pos]=='/' && li->buf[li->pos+1]=='/')
	{
		while (li->buf[li->pos]!=10 && li->buf[li->pos]!=13
			&& li->buf[li->pos]!=0)
			li->pos++;
		goto l1;
	}
	/* multi-line comments */
	if (li->buf[li->pos]=='/' && li->buf[li->pos+1]=='*')
	{
		li->pos += 2;
		while (!(li->buf[li->pos]=='*'&&li->buf[li->pos+1]=='/')
			&& li->buf[li->pos]!=0)
			if (!skipcr(li)) li->pos++;
		li->pos += 2;
		goto l1;
	}

	/* all other tokens, do the regex match when the first char matches */
	for (i=0, pos=0; i<sizeof(tokens)/sizeof(token_name); i++, pos=0)
		if (!li->regtrees[i].first ||
			*(li->buf+li->pos+pos)==li->regtrees[i].first)
			if (regmatch(li->buf+li->pos, &pos, li->regtrees[i].reg))
				break;
	if (i == sizeof(tokens)/sizeof(token_name))	/* match no lex rule */
	{
		li->pos++;
		return UKERROR;
	}
	li->next_token.token = tokens[i].token;
	memcpy(li->next_token.dbuf, li->buf+li->pos, sizeof(char)*pos);
	li->next_token.dbuf[pos] = 0;
	if (li->next_token.token == IDENTIFIER)
		li->next_token.token = clReservedLookup(li->next_token.dbuf);
	li->pos += pos;
	return li->token.token;
}
