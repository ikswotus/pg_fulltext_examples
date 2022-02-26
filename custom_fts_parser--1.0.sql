-- CREATE EXTENSION treats \ as comments, psql will try to execute and fail
\echo Use "CREATE EXTENSION custom_fts_parser;" to load this file. \quit

-- Parser Functions

CREATE OR REPLACE FUNCTION cfts_parser_start(internal, integer)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cfts_parser_nexttoken(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cfts_parser_end(internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cfts_parser_lextype(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;


-- Dictionary Functions


CREATE OR REPLACE FUNCTION cfts_tokendict_init(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cfts_tokendict_lexize(internal, internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;


-- Create a parser/configuration
CREATE TEXT SEARCH PARSER cfts_parser (
	START = cfts_parser_start,
	GETTOKEN = cfts_parser_nexttoken,
	END = cfts_parser_end,
	LEXTYPES = cfts_parser_lextype
);
COMMENT ON TEXT SEARCH PARSER cfts_parser IS 'custom text parser';

CREATE TEXT SEARCH CONFIGURATION cfts (
	PARSER = "cfts_parser"
);

ALTER TEXT SEARCH CONFIGURATION cfts ADD MAPPING for word with simple;

COMMENT ON TEXT SEARCH CONFIGURATION cfts IS 'cfts custom parser configuration';

-- Create some dictionaries

CREATE TEXT SEARCH TEMPLATE cfts_dict (
	INIT = cfts_tokendict_init,
	LEXIZE = cfts_tokendict_lexize
);
COMMENT ON TEXT SEARCH TEMPLATE cfts_dict IS 'cfts dictionary: custom subtokens based on underscores';

CREATE TEXT SEARCH DICTIONARY cfts_both (
	TEMPLATE = cfts_dict,
	tokenize_full = true,
	tokenize_prefix = true
);
COMMENT ON TEXT SEARCH DICTIONARY cfts_both IS 'cfts dictionary with tokenize_full = true, tokenize_prefix = true';


CREATE TEXT SEARCH DICTIONARY cfts_prefix (
	TEMPLATE = cfts_dict,
	tokenize_full = false,
	tokenize_prefix = true
);
COMMENT ON TEXT SEARCH DICTIONARY cfts_prefix IS 'cfts dictionary with tokenize_full = false, tokenize_prefix = true';
