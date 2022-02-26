CREATE EXTENSION IF NOT EXISTS custom_fts_parser;

-- See what our parser does, pretty much the same as default but without blank tokens.
select * from ts_debug('cfts', 'C:\Program Files\PostgreSQL\13\share\extension');

-- Parser tests/examples to verify tokens for our target conditions
select * from ts_lexize('cfts_both', 'fn12345_middletoken_junktextatend');
select * from ts_lexize('cfts_prefix', 'fn12345_middletoken_junktextatend');

select * from ts_lexize('cfts_both', 'notfn_soweexpectfulltext_here');

select * from ts_lexize('cfts_both', 'fn_too_short');

select * from ts_lexize('cfts_both', 'fn_butnotenoughunderscorestobeuseful');