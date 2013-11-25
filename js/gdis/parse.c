/*
Copyright (C) 2003 by Sean David Fleming

sean@ivec.org

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

The GNU GPL can also be found at http://www.gnu.org
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#ifndef __WIN32
#include <termios.h>
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef __WIN32
#include <sys/param.h>
#include <sys/wait.h>
#endif

#include "gdis.h"
#include "file.h"
#include "parse.h"
#include "matrix.h"
#include "import.h"
#include "keywords.h"
#include "interface.h"

/* main structures */
extern struct sysenv_pak sysenv;
extern struct elem_pak elements[];

/***********************/
/* strip off extension */
/***********************/
gchar *parse_strip_extension(const gchar *name)
{
gint i;
gchar *base, *temp;

temp = g_strdup(name);

/* get the rightmost '.' */
for (i=strlen(temp) ; i-- ; )
  if (*(temp+i) == '.')
    {
    base = g_strndup(temp, i);
    g_free(temp);
    return(base);
    }

/* not found - return the whole thing */
return(temp);
}

/***************************/
/* strip newline character */
/***************************/
/* NB: returns the first line in text, minus the newline char, and guarantee \0 termination */
gchar *parse_strip_newline(const gchar *text)
{
gint i;

for (i=0 ; i<strlen(text) ; i++)
  {
  if (text[i] == '\n')
    return(g_strndup(text, i));
  }

return(g_strdup(text));
}

/*****************************************/
/* strip off path and extension (if any) */
/*****************************************/
gchar *parse_strip(const gchar *name)
{
gint i;
gchar *base, *temp;

temp = g_path_get_basename(name);

/* get the rightmost '.' */
for (i=strlen(temp) ; i-- ; )
  if (*(temp+i) == '.')
    {
    base = g_strndup(temp, i);
    g_free(temp);
    return(base);
    }

/* not found - return the whole thing */
return(temp);
}

/**********************************************/
/* enforce (replace or add) a given extension */
/**********************************************/
gchar *parse_extension_set(const gchar *text, const gchar *ext)
{
gint i;
gchar *base, *name;

/* get the rightmost '.' */
for (i=strlen(text) ; i-- ; )
  if (*(text+i) == '.')
    {
/* replace extension */
    base = g_strndup(text, i);
    name = g_strdup_printf("%s.%s", base, ext);
    g_free(base);
    return(name);
    }

/* no extension found - add the extension and return */
name = g_strdup_printf("%s.%s", text, ext);
return(name);
}

/**************************************/
/* search for a character in a string */
/**************************************/
gchar *find_char(const gchar *text, gint target, gint search_type)
{
gint i;

switch(search_type)
  {
  case LAST:
    for (i=strlen(text) ; i-- ; )
      if (text[i] == target)
        return((gchar *) text+i);
    break;
  default:
    g_assert_not_reached();
  }
return(NULL);
}

/******************************/
/* count character occurances */
/******************************/
gint char_count(const gchar *text, gchar c)
{
gint i, n=0;

for (i=strlen(text) ; i-- ; )
  if (text[i] == c)
    n++;
return(n);
}

/*************************************************************/
/* replace all space characters in a string with a character */
/*************************************************************/
void parse_space_replace(gchar *text, gchar c)
{
gchar *t = text;

while (*t++)
  {
  switch (*t)
    {
/* exceptions */
    case EOF:
      return;
    case '\r':
    case '\n':
      break;

    default:
      if (g_ascii_isspace(*t))
        *t = c;
    }
  }
}

/******************************************************************************/
/* remove all non-alphanumeric chars before and after (but not in) the string */
/******************************************************************************/
void strip_extra(gchar *text)
{
gint i, n;

n = strlen(text);

/* white out preceding */
for (i=0 ; i<n ; i++)
  {
  if (g_ascii_isalnum(text[i]))
    break;
  else
    text[i] = ' ';
  }
/* white out trailing */
for (i=n ; i-- ; )
  {
  if (g_ascii_isalnum(text[i]))
    break;
  else
    text[i] = ' ';
  }
g_strstrip(text);
}

/********************************************/
/* test if a text string looks like a float */
/********************************************/
#define DEBUG_STR_IS_FLOAT 0
gint str_is_float(const gchar *text)
{
gint i, n;

#if DEBUG_STR_IS_FLOAT
printf("testing [%s] ", text);
#endif

n=0;
for (i=0 ; i<strlen(text) ; i++)
  {
  switch(text[i])
    {
/* exceptions */
    case '+':
    case '-':
    case '.':
      break;

/* cope with exp notation chars */
    case 'D':
    case 'E':
      n++;
      break;

    default:
      if (g_ascii_isdigit(text[i]))
        break;
#if DEBUG_STR_IS_FLOAT
printf(" = not a number (contains %c)\n", text[i]);
#endif
      return(FALSE);
    }

  if (n > 1)
    {
#if DEBUG_STR_IS_FLOAT
printf(" = not a number (contains more than one exponential character)\n");
#endif
    return(FALSE);
    }
  }
#if DEBUG_STR_IS_FLOAT
printf(" = a number.\n");
#endif
return(TRUE);
}

/*******************************************************/
/* get a float from a string (even in fraction format) */
/*******************************************************/
/* TODO - fortran prints a bunch of *'s if the number is too big - */
/* find a way to cope with this */
#define DEBUG_STR_TO_FLOAT 0
gdouble str_to_float(const gchar *txt)
{
gint i, j=0;
gchar *str;
gdouble den, val;

/* return 0.0 for NULL string */
if (txt == NULL)
  return(0.0);

/* if we have a backslash, process number as a fraction */
str = g_strdup(txt);

#if DEBUG_STR_TO_FLOAT
printf("[%s] : ", str);
#endif

/* scan string and do any substitutions */
for (i=strlen(str) ; i-- ; )
  {
  switch (*(str+i))
    {
/* process as fraction? */
    case '/':
      if (i < strlen(str)-1)
        j = i+1;
      break;
/* remove any equal signs */
    case '=':
      *(str+i) = ' ';
      break;
/* Gaussian uses D instead of E in scientific notation - convert back */
    case 'd':
    case 'D':
      *(str+i) = 'E';
      break;
      }
    }
/* main conversion */
val = g_ascii_strtod(str, NULL);

/* fraction processing */
if (j)
  {
  den = g_ascii_strtod(str+j, NULL);
  if (den != 0.0)
    val /= den;
  }

#if DEBUG_STR_TO_FLOAT
printf(" (%f)\n", val);
#endif

g_free(str);
return(val);
}

/* Return a list of keywords found */
/* NEW - match all keywords, not just space separated ones */
#define DEBUG_GET_KEYWORDS_ANYWHERE 0
GSList *get_keywords_anywhere(gchar *str)
{
gint i, j=0;
GSList *list=NULL;

#if DEBUG_GET_KEYWORD_ANYWHERE
printf("extracted: ");
#endif

i = 0;
while (keywords[i].code != -1)
  {
  if (strstr(str, keywords[i].label) != NULL)
    {
#if DEBUG_GET_KEYWORD_ANYWHERE
printf(" %d",keywords[i].code);
#endif
    list = g_slist_prepend(list, GINT_TO_POINTER(keywords[i].code));
    j++;
    }
  i++;
  }
list = g_slist_reverse(list);

#if DEBUG_GET_KEYWORD_ANYWHERE
printf("\nKeywords found: %d\n", j);
#endif

return(list);
}

/* Return a list of keywords found (iff space separated!) */
/* NEW - replacement for below routine, avoids alloc/free worries */
#define DEBUG_GET_KEYWORDS 0
GSList *get_keywords(gchar *str)
{
gint i, j, n, len, num_tokens;
gchar **buff;
GSList *list=NULL;

#if DEBUG_GET_KEYWORDS
printf("extracting from: %s\n", str);
#endif

buff = tokenize(str, &num_tokens);

i=n=0;
while(i < num_tokens)
  {
  if (*(buff+i) == NULL)
    break;
/* default keyword code - nothing */
  j=0;
  while(keywords[j].code != -1)
    {
    len = strlen(keywords[j].label);
    if (g_ascii_strncasecmp(*(buff+i), keywords[j].label, len) == 0)
      {
#if DEBUG_GET_KEYWORDS
printf(" %d",keywords[j].code);
#endif
      list = g_slist_prepend(list, GINT_TO_POINTER(keywords[j].code));
      n++;
      }
    j++;
    }
  i++;
  }
list = g_slist_reverse(list);

g_strfreev(buff);
#if DEBUG_GET_KEYWORDS
printf("\nKeywords found: %d\n", n);
#endif

return(list);
}

/***********************************************************/
/* hash table function for comparing two character strings */
/***********************************************************/
gint hash_strcmp(gconstpointer a, gconstpointer b)
{
if (g_ascii_strcasecmp(a, b) == 0)
  return(TRUE);
return(FALSE);
}

/**************************************************************/
/* return a string of keyword code (if any) found in a string */
/**************************************************************/
/* 1st item -> number actually found */
#define DEBUG_GET_KEYWORD 0
gint *get_keyword(gchar *str, gint max)
{
gint i, j, n, len, num_tokens;
gchar **buff;
gint *list;

#if DEBUG_GET_KEYWORD
printf("extracted: ");
#endif

list = g_malloc((max+1) * sizeof(gint));
buff = tokenize(str, &num_tokens);

n=1;
i=0;
while(i < num_tokens)
  {
/* default keyword code - nothing */
  *(list+n) = -1;
  j=0;
  while(keywords[j].code != -1)
    {
    len = strlen(keywords[j].label);
    if (g_ascii_strncasecmp(*(buff+i), keywords[j].label, len) == 0)
      {
#if DEBUG_GET_KEYWORD
printf(" %d",keywords[j].code);
#endif
      *(list+n) = keywords[j].code;
      if (++n == max+1)
        goto get_keyword_done;
      }
    j++;
    }
  i++;
  }
get_keyword_done:;
g_strfreev(buff);
*list = n-1;
#if DEBUG_GET_KEYWORD
printf("\n");
#endif

return(list);
}

gint num_keys(void)
{
gint n;

n=0;
while (keywords[n].code != -1)
  n++;
/*
printf("Found %d keywords\n",n);
*/
return(n);
}

/*****************************************/
/* get a token's keyword number (if any) */
/*****************************************/
gint get_keyword_code(const gchar *token)
{
gint j, len;

j=0;
while(keywords[j].code != -1)
  {
  len = strlen(keywords[j].label);
  if (g_ascii_strncasecmp(token, keywords[j].label, len) == 0)
    return(j);
  j++;
  }
return(-1);
}

/*********************/
/* tokenize a string */
/*********************/
/* replacement routine for get_tokens() */
/* will get as many tokens as available (no more messing with MAX_TOKENS) */
#define DEBUG_TOKENIZE 0
gchar **tokenize(const gchar *src, gint *num)
{
gint i, j, n, len;
gchar *tmp, *ptr;
gchar **dest;
GSList *list=NULL, *item=NULL;

/* checks */
if (!src)
  {
  *num=0;
  return(NULL);
  }

/* duplicate & replace all whitespace with a space */
tmp = g_strdup(src);
for (i=0 ; i<strlen(tmp) ; i++)
  if (isspace((int) *(tmp+i)))
    *(tmp+i) = ' ';

/* strange errors can be avoided if a strstrip is done */
g_strstrip(tmp);

#if DEBUG_TOKENIZE
printf("tokenizing [%s]\n", tmp);
#endif

len = strlen(tmp);
i=n=0;
while(i<len)
  {
/* find end of current token */
  j=i;
  while(!isspace((int) *(tmp+j)) && j<len)
    j++;

/* assign token */
  ptr = g_strndup(tmp+i, j-i);

  list = g_slist_prepend(list, ptr);
  n++;

/* find start of new token */
  i=j;

  while(isspace((int) *(tmp+i)) && i<len)
    i++;
  }
list = g_slist_reverse(list);

/* return a NULL if no tokens were found */
if (!n)
  {
  *num = 0;
  g_free(tmp);
  free_slist(list);
  return(NULL);
  }

/* num+1 -> last ptr is NULL, so g_strfreev works */
dest = g_malloc((n+1)*sizeof(gchar *));

i=0;
/* fill in the non empty tokens */
item = list;
while (i<n)
  {
  if (item != NULL)
    {
/* comment character - ignore all subsequent tokens */
    ptr = item->data;
    if (*ptr == '#')
      break;

    *(dest+i) = g_strdup(ptr);
#if DEBUG_TOKENIZE
printf(" (%s)", ptr);
#endif
    item = g_slist_next(item);
    }
  else
    {
/* fake item */
    *(dest+i) = g_strdup(" ");;
#if DEBUG_TOKENIZE
printf(" (empty token)");
#endif
    }
  i++;
  }

/* terminate */
*(dest+i) = NULL;
*num = i;

#if DEBUG_TOKENIZE
printf(" %p",*(dest+i));
printf(": found %d tokens\n", *num);
#endif

/* done */
g_free(tmp);
free_slist(list);

return(dest);
}

/************************************************/
/* get the next (non-trivial) line and tokenize */
/************************************************/
/* NULL is returned on EOF */
gchar **get_tokenized_line(FILE *fp, gint *num_tokens)
{
gchar **buff, line[LINELEN];

do
  {
  if (fgetline(fp, line))
    {
    *num_tokens = 0;
    return(NULL);
    }
  buff = tokenize(line, num_tokens);
  }
while (!buff);

return(buff);
}

/*********************/
/* tokenize a string */
/*********************/
/* can be useful if you want a definite number of tokens */
/* also, doesnt skip # lines (useful for processing GROMACS stuff) */
#define DEBUG_GET_TOKENS 0
gchar **get_tokens(gchar *src, gint num)
{
gint i, j;
gchar **buff, **dest, *tmp;

/* duplicate & replace all whitespace with a space */
/* strange errors can be avoided if a strstrip is done */
tmp = g_strdup(src);
for (i=0 ; i<strlen(tmp) ; i++)
  if (isspace((int) *(tmp+i)))
    *(tmp+i) = ' ';
g_strstrip(tmp);

/* NB: most problems have occured by making MAX_TOKENS too small */
/* for some reason it can need many more than it would apparently seem */
buff = g_strsplit(tmp, " ", MAX_TOKENS);

/* num+1 -> last ptr is NULL, so g_strfreev works */
dest = g_malloc((num+1)*sizeof(gchar *));

i=j=0;
/* fill in the non empty tokens */
while (*(buff+i) != NULL && j<num)
  {
  if (strlen(*(buff+i)))
    *(dest+j++) = g_strdup(g_strstrip(*(buff+i)));
  i++;
  }

/* pad with empty strings */
while (j<num)
  *(dest+j++) = g_strdup("");

/* terminate */
*(dest+num) = NULL;

#if DEBUG_GET_TOKENS
for (i=0 ; i<num ; i++)
  printf("%s:",*(dest+i));
printf("%p\n",*(dest+num));
#endif

/* done */
g_strfreev(buff);
g_free(tmp);

return(dest);
}

/* need another routine that gets everything in a line past */
/* a specified point - this will replace copy_items(...ALL) */
gchar *get_token_pos(gchar *src, gint num)
{
gint i,j,n,len;

/* flag the start(i) and end(j) */
len = strlen(src);
i = j = 0;
for (n=0 ; n<=num ; n++)
  {
  i = j;
/* FIXME - use the isspace function here */
  while((*(src+i) == ' ' || *(src+i) == '\t') && i<len)
    i++; 

  j = i;
  while(*(src+j) != ' ' && *(src+j) != '\t' && j<len)
    j++;
  }
 
/* return ptr to position */
return(src+i);
}

/************************************************/
/* allocate a copy of the nth token in a string */
/************************************************/
// result should be freed when finished
gchar *parse_nth_token_dup(gint n, const gchar *line)
{
gint num_tokens;
gchar *token=NULL, **buff;

/* checks */
if (!line)
  return(NULL);
if (n < 0)
  return(NULL);

buff = tokenize(line, &num_tokens);
if (n < num_tokens)
  token = g_strdup(buff[n]);
g_strfreev(buff);

return(token);
}

/*****************************************************************/
/* get a character from the terminal, preventing it being echoed */
/*****************************************************************/
gint parse_getchar_hidden(void)
{
gint c=-1;
#ifndef __WIN32
struct termios old;
struct termios new;

/* get terminal stdin attributes */
if (tcgetattr(0, &old) == -1)
  return(-1);

/* turn off echo */
new = old;
new.c_lflag &= ~(ICANON | ECHO);
new.c_cc[VMIN] = 1;
/* don't timeout? */
new.c_cc[VTIME] = 0;

/* change the terminal attributes immediately */
if (tcsetattr(0, TCSANOW, &new) == -1)
  return(-1);

/* read in a character */
c = getchar();

/* restore terminal attributes */
tcsetattr(0, TCSANOW, &old);
#endif

return(c);
}

/******************************************************************/
/* Get a line of text, preventing it being echoed to the terminal */
/******************************************************************/
gchar *parse_getline_hidden(void)
{
gchar c;
GString *line;

line = g_string_new(NULL);

for (;;)
  {
  c = parse_getchar_hidden(); 

/* to display or not to display (more secure not to - but gives no feedback) */
//  printf("*");

  if (c == '\n')
    {
    printf("\n");
    break;
    }
  else
    g_string_sprintfa(line, "%c", c);
  }

return(g_string_free(line, FALSE));
}

/*********************************************************************/
/* parse a string for 3 integers, assumed to be hours, mins, seconds */
/*********************************************************************/
/* return time in seconds */
gint parse_time_hms(const gchar *text)
{
gint i, j, k, n, time=0;
gdouble t[3];

//printf("parse_time_hms(): %s\n", text);

j = strlen(text);
n = 0;
VEC3SET(t, 0.0, 0.0, 0.0);

while (j)
  {
  if (g_ascii_isdigit(text[j]))
    {
    i=j-1;
    while (i >= 0)
      {
      if (g_ascii_isdigit(text[i]))
        i--;
      else
        break;
      }
    
//    printf("integer: ");

t[n++] = str_to_float(&text[i+1]);
if (n > 2)
  break;

/*
    for (k=i+1 ; k<j+1 ; k++)
      {
      printf("%c", text[k]);
      }
    printf("\n");
*/

    j=i;
    }
  j--;
  }

time = t[2]*3600.0 + t[1]*60.0 + t[0];

//printf("%f hours , %f mins, %f secs\n", t[2], t[1], t[0]);
//printf("total = %d\n", time);

return(time);
}


/* NEW - line parse primitive */
/* hash lookup + method of dealing with keyword if found */
/* + index of item -> preserve input/output order */
struct item_pak
{
gint index;
gint method;
gpointer variable;
const gchar *label;
};

struct parse_pak
{
gint size;
GHashTable *table;
};

/**********************/
/* new parsing object */
/**********************/
gpointer parse_new(void)
{
struct parse_pak *parse;

parse = g_malloc(sizeof(struct parse_pak));

parse->size = 0;
parse->table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

return(parse);
}

/*************************/
/* destruction primitive */
/*************************/
void parse_free(gpointer data)
{
struct parse_pak *parse=data;

g_assert(parse != NULL);

g_hash_table_destroy(parse->table);
}

/******************************/
/* keyword addition primitive */
/******************************/
void parse_keyword_add(const gchar *keyword, gint method, gpointer p2v, gpointer data)
{
struct parse_pak *parse=data;
struct item_pak *item;

g_assert(parse != NULL);

item = g_malloc(sizeof(struct item_pak));

item->index = parse->size++;
item->method = method;
item->variable = p2v;
item->label = keyword;

/* ignore case for easier matching */
g_hash_table_insert(parse->table, g_ascii_strdown(keyword, -1), item);
}

/****************/
/* parse action */
/****************/
/* returns TRUE if line was recognized by the parse object, else false */
gint parse_line(const gchar *line, gpointer data)
{
gint found=FALSE, num_tokens;
gchar **buff, *new, *key;
struct item_pak *item;
struct parse_pak *parse=data;

g_assert(parse != NULL);

buff = tokenize(line, &num_tokens);

if (num_tokens > 1)
  {
  key = g_ascii_strdown(*buff, -1);
  item = g_hash_table_lookup(parse->table, key);
  g_free(key);
  if (item)
    {
    found = TRUE;

/* skip cases with NULL variable (ie ignore) */
if (item->variable)
  {

    switch (item->method)
      {
      case PARSE_FIRST_CHAR:
        new = g_strdup(*(buff+1));
/* move the address of the new string pointer to the address of the required string variable :-) */
/* NB: simple pointer assignment won't do the job here */
        memcpy(item->variable, &new, sizeof(gchar **));
        break;

      case PARSE_ALL_CHAR:
        new = g_strjoinv(" ", buff+1);
        memcpy(item->variable, &new, sizeof(gchar **));
        break;
      }

  }

    }
  }

g_strfreev(buff);

return(found);
}

/**************************/
/* reconstitute primitive */
/**************************/
void parse_line_inverse(gpointer key, gpointer value, gpointer data)
{
gchar **v1;
GString *buffer=data;
struct item_pak *item=value;

if (item->variable)
  {
  switch (item->method)
    {
    case PARSE_ALL_CHAR:
    case PARSE_FIRST_CHAR:
      v1 = item->variable;
      if (*v1)
        g_string_append_printf(buffer, "%s  %s\n", item->label, *v1);
      break;
    }
  }
}

/******************************************************************************/
/* sort the keywords by index (order they were entered into the parse struct) */
/******************************************************************************/
gint parse_index_sort(gpointer a, gpointer b)
{
struct item_pak *item1=a, *item2=b;

if (item1->index == item2->index)
  return(0);
if (item1->index > item2->index)
  return(1);

return(-1);
}

/*****************************************************************/
/* reconstitute the parsed text from the internal data structure */
/*****************************************************************/
gchar *parse_inverse(gpointer data)
{
gchar **v1;
GList *item, *list;
GString *buffer;
struct parse_pak *parse=data;

buffer = g_string_new(NULL);

/*  sort the key before output */
//list = g_hash_table_get_keys(parse->table);
list = g_hash_table_get_values(parse->table);
list = g_list_sort(list, (gpointer) parse_index_sort);

for (item=list ; item ; item=g_list_next(item))
  {
  struct item_pak *entry=item->data;

//  printf("[%d][%s]\n", entry->index, entry->label);

  if (entry->variable)
    {
    switch (entry->method)
      {
      case PARSE_ALL_CHAR:
      case PARSE_FIRST_CHAR:
        v1 = entry->variable;
        if (*v1)
          g_string_append_printf(buffer, "%s  %s\n", entry->label, *v1);
        break;
      }
    }
  }

g_list_free(list);

return(g_string_free(buffer, FALSE));
}

