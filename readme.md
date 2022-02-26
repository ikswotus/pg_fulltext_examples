A simple example of a custom parser/dictionary for use in postgres full-text searching.
This does not do anything fancy, just does the bare minimum to produce working implementations.

# Useful Links/Other Parsers:

https://www.postgresql.org/docs/14/textsearch-intro.html#TEXTSEARCH-DOCUMENT

http://www.sai.msu.su/~megera/postgres/fts/fts.pdf

https://www.highgo.ca/2020/05/15/build-postgresql-and-extension-on-windows/

https://postgresql.verite.pro/blog/2019/03/28/tsearch-dictionary.html

https://github.com/postgrespro/tsexact

# Visual Studio Setup:

General -> Configuration Type = Dynamic Library .dll
C/C++ -> Additional Include Directories = postgres/include
      -> Code Generation -> Enable C++ Exceptions = no
	  -> Advanced -> Compile As -> C Code /TC
Linker ->  Input -> Additional Dependencies -> postgres.lib
           Manifest File -> No
		   Additional Directories -> postgres/lib


# Installation
custom_fts_parser.dll goes in 'C:\Program Files\PostgreSQL\13\lib'
custom_fts_parser.control/.sql go in 'C:\Program Files\PostgreSQL\13\share\extension'
