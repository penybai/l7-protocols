/* Reads in up to MAX bytes and runs regcomp against them TIMES times, using
the regular expression given on the command line.

Uses the standard GNU regular expression library which the userspace version of
l7-filter uses.

See ../LICENCE for copyright
*/

#include <ctype.h>
#include <stdio.h>
#include <regex.h>

#define MAX 1500
#define TIMES 10000
#define MAX_PATTERN_LEN 8192

static int hex2dec(char c)
{
        switch (c)
        {
                case '0' ... '9':
                        return c - '0';
                case 'a' ... 'f':
                        return c - 'a' + 10;
                case 'A' ... 'F':
                        return c - 'A' + 10;
                default:
			fprintf(stderr, "hex2dec: bad value!\n");
                        exit(1);
        }
}

/* takes a string with \xHH escapes and returns one with the characters
they stand for */
static char * pre_process(char * s)
{
        char * result = malloc(strlen(s) + 1);
        int sindex = 0, rindex = 0;
        while( sindex < strlen(s) )  
        {
            if( sindex + 3 < strlen(s) &&
                s[sindex] == '\\' && s[sindex+1] == 'x' &&
                isxdigit(s[sindex + 2]) && isxdigit(s[sindex + 3]) )
                {
                        /* carefully remember to call tolower here... */
                        result[rindex] = tolower( hex2dec(s[sindex + 2])*16 +
                                                  hex2dec(s[sindex + 3] ) );
                        sindex += 3; /* 4 total */
                }
                else 
                        result[rindex] = tolower(s[sindex]);

                sindex++;  
                rindex++;
        }
        result[rindex] = '\0';

        return result;
}


void doit(regex_t * pattern, char ** argv, int verbose)
{
	char input[MAX];
	int c;

	for(c = 0; c < MAX; c++)
	{
		char temp = 0;
		while(temp == 0)
		{
			if(EOF == scanf("%c", &temp))
				goto out;
			input[c] = temp;
		}
	}
	out:

	input[c-1] = '\0';

	for(c = 0; c < MAX; c++) input[c] = tolower(input[c]);

	for(c = 1; c < TIMES; c++){
		int result = regexec(pattern, input, 0, 0, 0);
		if(c == 1)
			if(!result)
				fprintf(stderr, "match\t");
			else
				fprintf(stderr, "no_match\t");

		if(c%(TIMES/20) == 0){ fprintf(stderr, "."); }
	}
	if(verbose)
		puts("");
	else
		printf(" ");
}

// Syntax: test_speed regex [verbose]
int main(int argc, char ** argv)
{
	regex_t pattern;
	char * s = argv[1];
	int patternlen, i, verbose = 0;

	if(argc < 2){
		fprintf(stderr, "need an arg\n");
		return 1;
	}
	if(argc > 2)
		verbose = 1;

	patternlen = strlen(s);
	if(patternlen > MAX_PATTERN_LEN){
		fprintf(stderr, "Pattern too long! Max is %d\n", MAX_PATTERN_LEN);
		return 1;
	}

	s = pre_process(s); /* do \xHH escapes */

	if(regcomp(&pattern, s, REG_EXTENDED)){
		fprintf(stderr, "error compiling regexp\n");
		exit(1);
	}

	if(verbose)
		printf("running regexec \"%.16s...\" %d times\n", argv[1], TIMES);

	doit(&pattern, argv, verbose);

	return 0;
}
