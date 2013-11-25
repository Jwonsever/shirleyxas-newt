#include <stdio.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

/* TODO - this will be where the raw XML from get/post is handled wrt the rest interface */
/* see grisu_rest.c for where the xml is parsed into appropriate c structures */

//see http://curl.haxx.se/libcurl/c/libcurl-tutorial.html
// for more info (eg http POST ... bin file upload etc)
// also - progress meter!

int main(int argc, char **argv)
{
  CURL *curl;
  CURLcode res;
  FILE *headerfile;
  const char *pPassphrase = NULL;
  static const char *pCertFile = "/home/sean/.globus-slcs/usercert.pem";
  static const char *pCACertFile="/home/sean/.globus-slcs/usercert.pem";
  const char *pKeyName = "/home/sean/.globus-slcs/userkey.pem";
  const char *pKeyType = "PEM";
  const char *pEngine = NULL;

// NB: see curl_global_init() - CURL_GLOBAL_WIN32 
  curl = curl_easy_init();
  if(curl) {


//    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/soap/GrisuService");

// step 1 - this works (no auth required)
//    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/rest/grisu/info/allSites");

// command line - fails - server complains it couldnt get a myproxy cred
// might be because cert auth is required

// COMMANDLINE - THIS WORKS - (prompts for passphrase) - if there is a myproxy cred available
//curl -k --basic -u someguy "https://grisu-vpac.arcs.org.au/grisu-ws/rest/grisu/user/fqans"
// step 2 - auth requires consumption
//    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/rest/grisu/user/fqans");

// needed to auth server (can turn off - but a bit insecure)
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/home/sean/.globus/certificates");

    curl_easy_setopt(curl, CURLOPT_USERNAME, "pmckhabiil");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "d8wsdhhcu3oc");

/*
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
*/
//    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/rest/grisu/user/fqans");
//    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/rest/grisu/user/alljobnames/gulp");

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "jobname=gulp_newtest1");

    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/rest/grisu/user/getJobDetails");

// NB: easy_perform can be executed multiple times & it caches the session
    res = curl_easy_perform(curl);





    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }

printf("\n\n");

  return 0;


/*
  headerfile = fopen("dumpit", "w");

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://grisu-vpac.arcs.org.au/grisu-ws/soap/GrisuService");
    curl_easy_setopt(curl, CURLOPT_WRITEHEADER, headerfile);

    while(1)
    {
      if (pEngine)
      {
        if (curl_easy_setopt(curl, CURLOPT_SSLENGINE,pEngine) != CURLE_OK)
        {     
          fprintf(stderr,"can't set crypto engine\n");
          break;
        }
        if (curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT,1L) != CURLE_OK)
        { 
          fprintf(stderr,"can't set crypto engine as default\n");
          break;
        }
      }
      curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");

      curl_easy_setopt(curl,CURLOPT_SSLCERT,pCertFile);

      if (pPassphrase)
        curl_easy_setopt(curl,CURLOPT_KEYPASSWD,pPassphrase);

      curl_easy_setopt(curl,CURLOPT_SSLKEYTYPE,pKeyType);

      curl_easy_setopt(curl,CURLOPT_SSLKEY,pKeyName);

      curl_easy_setopt(curl,CURLOPT_CAINFO,pCACertFile);

      curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);

      res = curl_easy_perform(curl);
      break;  
    }
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
*/
}
