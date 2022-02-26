#include "postgres.h"
#include "commands/defrem.h"
#include "tsearch/ts_public.h"
#include "tsearch/ts_locale.h"

// Postgres will look for this 'magic block'
PG_MODULE_MAGIC;

/* declare our exposed functions required for a parser*/
PGDLLEXPORT Datum cfts_parser_start(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum cfts_parser_nexttoken(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum cfts_parser_end(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum cfts_parser_lextype(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(cfts_parser_start);
PG_FUNCTION_INFO_V1(cfts_parser_nexttoken);
PG_FUNCTION_INFO_V1(cfts_parser_end);
PG_FUNCTION_INFO_V1(cfts_parser_lextype);

/**
 * Our internal parsing state
 * Track the start,end and current position
 **/
typedef struct
{
	char* start;
	char* end;
	char* position;
} InternalParserState;

// supported token types - just do words
static const char* const token_types[] =
{
	"word",
};

// Description of supported tokens
static const char* const lex_descr[] =
{
	"Word, any nonslash/space characters",
};

/**
* Called to initialize our parser.
*
* Arguments are the text itself (0) and the length of the text (1)
*
* Returns a pointer to our internal state
*/
Datum
cfts_parser_start(PG_FUNCTION_ARGS)
{
	InternalParserState* status = (InternalParserState*)palloc0(sizeof(InternalParserState));

	status->start = (char*)PG_GETARG_POINTER(0);
	status->end = status->start + PG_GETARG_INT32(1);
	status->position = status->start;

	PG_RETURN_POINTER(status);
}

/**
* Parsing is over
*
* Args[0] = our parser
*
* Just needs to release our resources
*/
Datum
cfts_parser_end(PG_FUNCTION_ARGS)
{
	InternalParserState* status = (InternalParserState*)PG_GETARG_POINTER(0);

	pfree(status);

	PG_RETURN_VOID();
}

/**
* Retrieve the next token. Will be called while we return a token type other than 0
* Arguments will be:
* [0] Our internal parser state
* [1] out - token start pointer
* [2] out - token length pointer
*
* Returns a token type if a token was parsed, or 0
*
* This parser will split based on slashes or spaces
*/
Datum
cfts_parser_nexttoken(PG_FUNCTION_ARGS)
{
	InternalParserState* status = (InternalParserState*)PG_GETARG_POINTER(0);

	char** token = (char**)PG_GETARG_POINTER(1);
	int* token_length = (int*)PG_GETARG_POINTER(2);

	int type = 0;

	// Locate the first non-slash, non-space char
	while (status->position < status->end && (*status->position == '\\' || *status->position == ' '))
	{
		int p_len = pg_mblen(status->position);
		status->position += p_len;
	}
	// Do we have data left?
	if (status->position < status->end)
	{
		int p_len = pg_mblen(status->position);

		type = 1;
		*token = status->position;

		status->position += p_len;

		// Consume up to next token
		while (status->position < status->end && (*status->position != '\\' && *status->position != ' '))
		{
			status->position += pg_mblen(status->position);
		}

		*token_length = (status->position - *token);

		// Skip small tokens for now, we could also rely on stop-words or a custom dictionary to avoid small entries
		if (*token_length < 2)
		{
			type = 0;
		}
	}

	PG_RETURN_INT32(type);
}

/**
* Identifies the token types this parser is capable of returning
*
* See /include/tsearch/ts_public.h
*/
Datum
cfts_parser_lextype(PG_FUNCTION_ARGS)
{
	// There's only 1 token type that's valid: word.
	// 2) will be the empty token
	LexDescr* descr = (LexDescr*)palloc(sizeof(LexDescr) * (2));

	descr[0].lexid = 1;
	descr[0].alias = pstrdup(token_types[0]);
	descr[0].descr = pstrdup(lex_descr[0]);

	descr[1].lexid = 0;

	PG_RETURN_POINTER(descr);
}