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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include "gdis.h"
#include "numeric.h"

/**************************/
/* process x509 time data */
/**************************/
time_t time_from_asn1(char *asn1time)
{
char   zone;
struct tm time_tm;
int len;

/* checks */
len = strlen(asn1time);
if ((len != 13) && (len != 15))
  return 0;
                                                                                
if ((len == 13) &&
    ((sscanf(asn1time, "%02d%02d%02d%02d%02d%02d%c",
     &(time_tm.tm_year),
     &(time_tm.tm_mon),
     &(time_tm.tm_mday),
     &(time_tm.tm_hour),
     &(time_tm.tm_min),
     &(time_tm.tm_sec),
     &zone) != 7) || (zone != 'Z')))
  return 0;
                                                                                
if ((len == 15) &&
    ((sscanf(asn1time, "20%02d%02d%02d%02d%02d%02d%c",
     &(time_tm.tm_year),
     &(time_tm.tm_mon),
     &(time_tm.tm_mday),
     &(time_tm.tm_hour),
     &(time_tm.tm_min),
     &(time_tm.tm_sec),
     &zone) != 7) || (zone != 'Z')))
  return 0;

/* time format fixups */
if (time_tm.tm_year < 90) time_tm.tm_year += 100;
  --(time_tm.tm_mon);

return mktime(&time_tm);
}

/***********************************************/
/* scan a local certificate for time remaining */
/***********************************************/
// filename should ref a PEM certificate (eg slcs)
#define DEBUG_PEM_HOURS_LEFT 0
gdouble grid_pem_hours_left(const gchar *filename)
{
gchar *b;
gdouble delta=0.0;
time_t expire, current;
ASN1_TIME *t;
X509 *cert;
FILE *fp;

if (!filename)
  return(delta);

/* attempt to fill out struct with data from file */
fp = fopen(filename, "rb");
if (!fp)
  {
#if DEBUG_PEM_HOURS_LEFT
printf("Failed to open: %s\n", filename);
#endif
  return(-1.0);
  }
cert = X509_new();
if (PEM_read_X509(fp, &cert, NULL, NULL))
  {
#if DEBUG_PEM_HOURS_LEFT
  printf("Read certificate.\n");
#endif
  }
else
  {
#if DEBUG_PEM_HOURS_LEFT
  printf("Failed to read certificate.\n");
#endif
  return(-1.0);
  }

#if DEBUG_PEM_HOURS_LEFT
printf("Certificate: %s\n", filename);
#endif

// DEBUG
//X509_print_fp(stdout, cert);

/* compare expiry time with the current time */
t = X509_get_notAfter(cert);
b = (gchar *) ASN1_STRING_data(t);
expire = time_from_asn1(b);
if (!expire)
  return(delta);

#if DEBUG_PEM_HOURS_LEFT
printf("Certificate is valid until: %s\n", time_string(&expire));
#endif

time(&current);
delta = difftime(expire, current);

/* return 0 if time remaining is too small */
if (delta > 0.1)
  delta /= 3600.0;
else
  delta = 0.0;

#if DEBUG_PEM_HOURS_LEFT
printf("Hours left = %f\n", delta);
#endif

return(delta);
}

