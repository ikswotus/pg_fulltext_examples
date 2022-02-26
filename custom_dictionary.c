#include "postgres.h"
#include "commands/defrem.h"
#include "tsearch/ts_public.h"
#include "tsearch/ts_locale.h"

// Done in custom_parser.c, if we're only interested in a dictionary and not a full parser we can
// declare it here instead
//PG_MODULE_MAGIC;

/* Holds our options for the dictionary */
typedef struct
{
	// If true, a sub-token from 0 through the first '_' will be included
	bool tokenize_prefix;
	// If true, the full token is also returned.
	bool tokenize_full;
} CFTSDict;


PGDLLEXPORT Datum cfts_tokendict_init(PG_FUNCTION_ARGS);
PGDLLEXPORT Datum cfts_tokendict_lexize(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(cfts_tokendict_init);
PG_FUNCTION_INFO_V1(cfts_tokendict_lexize);

/**
* Initialize our CFTSDict with the token options
*
* Returns an initialized CFTSDict
*/
Datum
cfts_tokendict_init(PG_FUNCTION_ARGS)
{
	List* dictoptions = (List*)PG_GETARG_POINTER(0);
	CFTSDict* d = (CFTSDict*)palloc0(sizeof(CFTSDict));

	bool tp = false, tf = false;
	ListCell* l;

	foreach(l, dictoptions)
	{
		DefElem* defel = (DefElem*)lfirst(l);

		if (pg_strcasecmp("tokenize_prefix", defel->defname) == 0)
		{
			if (tp)
				ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("multiple tokeinze_prefix parameters")));
			d->tokenize_prefix = defGetBoolean(defel);
			tp = true;
		}
		else if (pg_strcasecmp("tokenize_full", defel->defname) == 0)
		{
			if (tf)
				ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						errmsg("multiple tokenize_full parameters")));

			d->tokenize_full = defGetBoolean(defel);
			tf = true;
		}
		else // Report any unknown options
		{
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					errmsg("unrecognized dictionary parameter: \"%s\"",
						defel->defname)));
		}
	}

	// Make sure both were set
	if (!tp || !tf)
	{
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("both tokenize options are required")));
	}

	PG_RETURN_POINTER(d);
}

/**
* Checks tokens for our target conditions.
*
* Current conditions are:
* token length > 20
* the token starts with 'fn'
* token contains at least two '_' characters
*
* Non-matching tokens are returned as-is.
*
* If the input token matches, we will subdivide the token
* based on our configured options, and return up to 3 tokens:
*
* 1) A sub-token is always created between the first '_' and the last '_' characters.
* 2) If tokenize_prefix is set, we create a sub-token from position 0 to the first '_'
* 3) If tokenize_full is set, we include the original full token.
*
* Example: if our token is 'fn_subtoken_junkatendofstringnotreturned' (40)
* we will return:
* [fn,subtoken,fn_subtoken_junkatendofstringnotreturned] if tokenize_full & tokenize_prefix are both true
* [subtoken] if both tokenize_full & tokenize_prefix are false
* [fn,subtoken] if tokenize_prefix is true, tokenize_full is false
* 
* Arguments:
* 0 Our dictionary object
* 1) Token
* 2) token length
*/
Datum
cfts_tokendict_lexize(PG_FUNCTION_ARGS)
{
	CFTSDict* d = (CFTSDict*)PG_GETARG_POINTER(0);
	char* token = (char*)PG_GETARG_POINTER(1);
	int32 token_length = PG_GETARG_INT32(2);

	// Validate some target conditions
	if (token_length < 20 || token[0] != 'f' || token[1] != 'n')
	{
		// Not sure why, but we always need to return 2 
		TSLexeme* res = palloc0(sizeof(TSLexeme) * 2);
		res[0].nvariant = 1;
		// Cant use pstrdup - doesnt know length
		res[0].lexeme = pnstrdup(token, token_length);
		PG_RETURN_POINTER(res);
	}

	char* su = token;
	char* act = token;
	char* eu = token;
	char* p = token;
	bool saw = false;

	for (int i = 0; i < token_length; i++)
	{
		p += pg_mblen(p);
		if (saw)
		{
			su = p;
			saw = false;
		}

		if (*p == '_')
		{

			if (su == token)
			{
				act = p;
				saw = true;
			}
			else
				eu = p;
		}

	}
	if (eu == token || su == token || su == eu)
	{
		TSLexeme* res = palloc0(sizeof(TSLexeme) * 2);
		res[0].nvariant = 1;
		res[0].lexeme = pnstrdup(token, token_length);
		PG_RETURN_POINTER(res);
	}

	// Met conditions - see what we need to return
	int tokens = 2;
	// This needs to be 1 more than the number of tokens we return...not entirely sure why, possibly
	// a null-terminator thing?
	// it will definitely crash postgres on the next call if we only allocate space for 1 token and return 1 token
	// and dict_simple.c sets size to *2 so 2 it is.

	if (d->tokenize_full)
		tokens++;
	if (d->tokenize_prefix)
		tokens++;

	TSLexeme* res = palloc0(sizeof(TSLexeme) * tokens);

	int rettok = 0;
	uint16 nvariant = 1;

	if (d->tokenize_full)
	{
		res[rettok].nvariant = nvariant;
		res[rettok++].lexeme = pnstrdup(token, token_length); 
	}

	if (d->tokenize_prefix)
	{
		res[rettok].nvariant = 2;
		res[rettok++].lexeme = pnstrdup(token, act - token);
	}

	res[rettok].nvariant = 2;
	res[rettok++].lexeme = pnstrdup(su, eu - su);


	PG_RETURN_POINTER(res);
}
