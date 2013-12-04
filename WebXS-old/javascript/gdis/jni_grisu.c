/*
Copyright (C) 2010 by Sean David Fleming

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

#include <jni.h>

#include "gdis.h"
#include "file.h"

/* globals */
extern struct sysenv_pak sysenv;
JavaVM *jvm=NULL;
JNIEnv *env=NULL;
// instanced SLCS object
jobject object_slcsclient=NULL;

/*********************/
/* exception handler */
/*********************/
#define DEBUG_JNI_EXCEPTION_HANDLE 1
void jni_exception_handle(void)
{
g_assert(env != NULL);

if ((*env)->ExceptionOccurred(env))
  {
#if DEBUG_JNI_EXCEPTION_HANDLE
(*env)->ExceptionDescribe(env);
#endif

  (*env)->ExceptionClear(env);
  }
}

/********************************************************/
/* convenience wrapper to convert c type to java char[] */
/********************************************************/
jcharArray jni_jcharArray_from_char(const gchar *input)
{
jstring tmp;
jsize len;
const jchar *buff;
jcharArray output;

/* convert char* to jstring */
tmp = (*env)->NewStringUTF(env, input);
len = (*env)->GetStringUTFLength(env, tmp);

/* jconvert string to jchar* */
buff = (*env)->GetStringChars(env, tmp, 0);

/* convert jchar* to jcharArray */
output = (*env)->NewCharArray(env, len);
(*env)->SetCharArrayRegion(env, output, 0, len, buff);

return(output);
}

/**********************************/
/* combined class and method seek */
/**********************************/
/* FIXME - need to differentiate between static/non-static as they are invoked differently */
/* ie (*env)->CallStaticObjectMethod(env, class, method); */
/* vs (*env)->CallObjectMethod(env, class, method); */

#define DEBUG_JNI_INIT_METHOD 1
gint jni_method_init(const gchar *class_name, const gchar *method_name, const gchar *method_signature,
                     jclass *class, jmethodID *method)
{
g_assert(env != NULL);

*class = (*env)->FindClass(env, class_name);
if (!*class)
  {
#if DEBUG_JNI_INIT_METHOD
printf("Can't find class: [%s]\n", class_name);
#endif
  }
else
  {
// attempt to get non-static method
  *method = (*env)->GetMethodID(env, *class, method_name, method_signature);
  if (*method) 
    return(TRUE);
  else
    {
// attempt to get static method
    *method = (*env)->GetStaticMethodID(env, *class, method_name, method_signature);
    if (*method)
      return(TRUE);

#if DEBUG_JNI_INIT_METHOD
printf("Can't find method: [%s] with signature: [%s]\n", method_name, method_signature);
#endif
    }
  }
return(FALSE);
}

/********/
/* stub */
/********/
/*
public final void setServiceInterfaceUrl(java.lang.String);
  Signature: (Ljava/lang/String;)V
*/
void jni_setServiceInterfaceUrl(const gchar *url) 
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "setServiceInterfaceUrl";
const gchar *method_signature = "(Ljava/lang/String;)V";
jclass class;
jmethodID method;
jstring arg1;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
// invoke
  arg1 = (*env)->NewStringUTF(env, url);
  (*env)->CallObjectMethod(env, class, method, arg1);
  }

jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_setMyProxyUsername(const gchar *username) 
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "setMyProxyUsername";
const gchar *method_signature = "(Ljava/lang/String;)V";
jclass class;
jmethodID method;
jstring arg1;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
// invoke
  arg1 = (*env)->NewStringUTF(env, username);
  (*env)->CallObjectMethod(env, class, method, arg1);
  }

jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_setMyProxyPassword(const gchar *password) 
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "setMyProxyPassphrase";
const gchar *method_signature = "([C)V";
jclass class;
jmethodID method;
jcharArray arg1;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  arg1 = jni_jcharArray_from_char(password);

  (*env)->CallObjectMethod(env, class, method, arg1);
  }

jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_setMyProxyServer(const gchar *server) 
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "setMyProxyServer";
const gchar *method_signature = "(Ljava/lang/String;)V";
jclass class;
jmethodID method;
jstring arg1;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
// invoke
  arg1 = (*env)->NewStringUTF(env, server);
  (*env)->CallObjectMethod(env, class, method, arg1);
  }

jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_setMyProxyPort(const gchar *port) 
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "setMyProxyPort";
const gchar *method_signature = "(Ljava/lang/String;)V";
jclass class;
jmethodID method;
jstring arg1;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
// invoke
  arg1 = (*env)->NewStringUTF(env, port);
  (*env)->CallObjectMethod(env, class, method, arg1);
  }

jni_exception_handle();
}

/********/
/* stub */
/********/
/*
public final java.lang.String getServiceInterfaceUrl();
  Signature: ()Ljava/lang/String;
*/

void jni_getServiceInterfaceUrl(void) 
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "getServiceInterfaceUrl";
const gchar *method_signature = "()Ljava/lang/String;";
jclass class;
jmethodID method;
jstring ret;
const gchar *val;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  ret = (*env)->CallObjectMethod(env, class, method);
  if (ret)
    {
    val = (*env)->GetStringUTFChars(env, ret, 0);

printf("service interface = %s\n", val);

    (*env)->ReleaseStringUTFChars(env, ret, val);
    }
  else
    printf("service interface not set.\n");
  }
jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_getMyProxyUsername(void)
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "getMyProxyUsername";
const gchar *method_signature = "()Ljava/lang/String;";
jclass class;
jmethodID method;
jstring ret;
const gchar *val;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  ret = (*env)->CallObjectMethod(env, class, method);
  if (ret)
    {
    val = (*env)->GetStringUTFChars(env, ret, 0);

printf("myproxy username = %s\n", val);

    (*env)->ReleaseStringUTFChars(env, ret, val);
    }
  else
    printf("myproxy username not set.\n");

  }
jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_getMyProxyPassword(void)
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "getMyProxyPassphrase";
const gchar *method_signature = "()[C";
jclass class;
jmethodID method;
//jcharArray tmp;
jobject tmp;
jsize len;
jstring string;
const gchar *val;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  tmp = (*env)->CallObjectMethod(env, class, method);
  if (tmp)
    {
// jsize GetArrayLength(jarray array)
//    jchar *val = (*env)->GetCharArrayElements(env, tmp, 0);
// I think the problem is jchar is 2 bytes ... char is 1 byte
// so the conversion can be tricky

    len = (*env)->GetArrayLength(env, tmp);

// type as jstring necessary since getobjarray returns jobject
    string = (jstring) (*env)->GetObjectArrayElement(env, tmp, len);

    val = (*env)->GetStringUTFChars(env, string, 0);


printf("myproxy pwd = %s\n", val);

/*
  jstring (JNICALL *NewString)
      (JNIEnv *env, const jchar *unicode, jsize len);
*/

/*
    val = (*env)->GetStringUTFChars(env, ret, 0);


    (*env)->ReleaseStringUTFChars(env, ret, val);
*/

/*
    jchar *val = (*env)->GetCharArrayElements(env, ret, 0);

printf("myproxy password = %s\n", val);

    (*env)->ReleaseCharArrayElements(env, ret, val, 0);
*/
    }
  else
    printf("myproxy password not set.\n");

  }
jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_getMyProxyServer(void)
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "getMyProxyServer";
const gchar *method_signature = "()Ljava/lang/String;";
jclass class;
jmethodID method;
jstring ret;
const gchar *val;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  ret = (*env)->CallObjectMethod(env, class, method);
  if (ret)
    {
    val = (*env)->GetStringUTFChars(env, ret, 0);

printf("myproxy server = %s\n", val);

    (*env)->ReleaseStringUTFChars(env, ret, val);
    }
  else
    printf("myproxy server not set.\n");

  }
jni_exception_handle();
}

/********/
/* stub */
/********/
void jni_getMyProxyPort(void)
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_name = "getMyProxyPort";
const gchar *method_signature = "()Ljava/lang/String;";
jclass class;
jmethodID method;
jstring ret;
const gchar *val;

g_assert(env != NULL);

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  ret = (*env)->CallObjectMethod(env, class, method);
  if (ret)
    {
    val = (*env)->GetStringUTFChars(env, ret, 0);

printf("myproxy port = %s\n", val);

    (*env)->ReleaseStringUTFChars(env, ret, val);
    }
  else
    printf("myproxy port not set.\n");

  }
jni_exception_handle();
}


/*
Stores credentials on MyProxy server.

org.globus.myproxy.MyProxy 

void	put(String host, int port, GSSCredential credential, String username, String passphrase, int lifetime) 


public static void put(java.lang.String, int, org.ietf.jgss.GSSCredential, java.lang.String, java.lang.String, int)   throws org.globus.myproxy.MyProxyException;
  Signature: (Ljava/lang/String;ILorg/ietf/jgss/GSSCredential;Ljava/lang/String;Ljava/lang/String;I)V

*/


void jni_myproxy_upload(jobject gss_credential)
{
jclass class;
jmethodID method;

/* TODO - common call for type checking */
/* TODO - check it's a GSSCredential */

/* init params method */
if (jni_method_init("org/globus/myproxy/MyProxy",
                    "put",
                    "(Ljava/lang/String;ILorg/ietf/jgss/GSSCredential;Ljava/lang/String;Ljava/lang/String;I)V",
                    &class, &method))
  {
  jstring server = (*env)->NewStringUTF(env, "myproxy.arcs.org.au");
  jint port = 7512;
  jstring u = (*env)->NewStringUTF(env, "newguy");
  jstring p = (*env)->NewStringUTF(env, "qetuo135");
  jint lifetime = 10000;

printf("Calling upload method...\n");

  (*env)->CallObjectMethod(env, class, method, server, port, gss_credential, u, p, lifetime);

printf("done\n");
  }


}



/********/
/* stub */
/********/
/*
public org.vpac.grisu.frontend.control.login.LoginParams(java.lang.String, java.lang.String, char[], java.lang.String, java.lang.String);
  Signature: (Ljava/lang/String;Ljava/lang/String;[CLjava/lang/String;Ljava/lang/String;)V
*/

/* set all login parameters in one fell swoop */
/*
void jni_loginParams(const gchar *si, const gchar *mpu, const gchar *mpp, const gchar *mps, const gchar *mpt)
{
const gchar *class_name = "org/vpac/grisu/frontend/control/login/LoginParams";
const gchar *method_signature = "(Ljava/lang/String;Ljava/lang/String;[CLjava/lang/String;Ljava/lang/String;)V";
jclass class;
jmethodID method;

if (jni_method_init(class_name, "<init>", method_signature, &class, &method))
  {
  jstring arg1, arg2, arg3, arg4;
  jobject lp;

printf("got constructor method.\n");

  lp = (*env)->NewObject(env, class, method, arg1, arg2, arg3, arg3);

  }

}
*/

/*
public GSSCredential slcsLogin(IDP idp, String username, String password,
           String myproxyUsername, String myproxyPassword)
*/

/*
GSSCredential cred = client.slcsLogin(idps.get(4), username, password);
public org.ietf.jgss.GSSCredential slcsLogin(au.edu.archer.desktopshibboleth.idp.IDP, java.lang.String, char[])   throws java.lang.Exception;
  Signature: (Lau/edu/archer/desktopshibboleth/idp/IDP;Ljava/lang/String;[C)Lorg/ietf/jgss/GSSCredential;

*/

/**********************************/
/* from the swiss-proxy-knife.jar */
/**********************************/
/*
org.vpac.security.light.CredentialHelpers

static void 	writeToDisk(org.ietf.jgss.GSSCredential gssCred)
          Writes a GSSCredential to the default globus location
*/

/*
public static void writeToDisk(org.ietf.jgss.GSSCredential)   throws java.io.IOException, org.ietf.jgss.GSSException;
  Signature: (Lorg/ietf/jgss/GSSCredential;)V
*/

void jni_store_gsscred(jobject gsscred)
{
const gchar *class_name = "org/vpac/security/light/CredentialHelpers";
const gchar *method_name = "writeToDisk";
const gchar *method_signature = "(Lorg/ietf/jgss/GSSCredential;)V"; 
jclass class;
jmethodID method;

if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
printf("Storing cred...\n");

  (*env)->CallObjectMethod(env, class, method, gsscred);

printf("ok\n");

/* CURRENT - fails - complains about no relevent signing policy for the arcs CA */
//jni_myproxy_upload(gsscred);
  }

}

/******************************/
/* plain credential retrieval */
/******************************/
void jni_login(jobject idp, const gchar *username, const gchar *password)
{
const gchar *method_name = "slcsLogin";
const gchar *method_signature = "(Lau/edu/archer/desktopshibboleth/idp/IDP;Ljava/lang/String;[C)Lorg/ietf/jgss/GSSCredential;";
jmethodID method;
jclass class;
jobject cred;

g_assert(env != NULL);
g_assert(object_slcsclient != NULL);

class = (*env)->GetObjectClass(env, object_slcsclient);

if (class)
  {
  method = (*env)->GetMethodID(env, class, method_name, method_signature);
  if (method)
    {
    jstring u = (*env)->NewStringUTF(env, username);
    jcharArray p = jni_jcharArray_from_char(password);

    cred = (*env)->CallObjectMethod(env, object_slcsclient, method, idp, u, p);

    if (cred)
      {
      jclass class_cred = (*env)->FindClass(env, "org/ietf/jgss/GSSCredential");

if (class_cred)
  {
  if ((*env)->IsInstanceOf(env, cred, class_cred) == JNI_TRUE)
    {
    printf("Got credential\n");

// try to write it out
jni_store_gsscred(cred);

    }
  }

    printf("Got something ...\n");
      }
    else
      printf("fail.\n");

    }
  }

jni_exception_handle();
}


/*******************************************/
/* credential retrieval and myproxy upload */
/*******************************************/
/*
public org.ietf.jgss.GSSCredential slcsLogin(au.edu.archer.desktopshibboleth.idp.IDP, java.lang.String, char[], java.lang.String, java.lang.String)   throws java.lang.Exception;
  Signature: (Lau/edu/archer/desktopshibboleth/idp/IDP;Ljava/lang/String;[CLjava/lang/String;Ljava/lang/String;)Lorg/ietf/jgss/GSSCredential;
*/
void jni_login2(jobject idp, const gchar *username, const gchar *password)
{
const gchar *method_name = "slcsLogin";
const gchar *method_signature = "(Lau/edu/archer/desktopshibboleth/idp/IDP;Ljava/lang/String;[CLjava/lang/String;Ljava/lang/String;)Lorg/ietf/jgss/GSSCredential;";
jmethodID method;
jclass class;
jobject cred;

g_assert(env != NULL);
g_assert(object_slcsclient != NULL);

class = (*env)->GetObjectClass(env, object_slcsclient);

if (class)
  {
  method = (*env)->GetMethodID(env, class, method_name, method_signature);
  if (method)
    {
    jstring u = (*env)->NewStringUTF(env, username);
    jcharArray p = jni_jcharArray_from_char(password);
    jstring mp_username = (*env)->NewStringUTF(env, "newguy");
    jstring mp_password = (*env)->NewStringUTF(env, "qetuo135");

    cred = (*env)->CallObjectMethod(env, object_slcsclient, method, idp, u, p, mp_username, mp_password);

    if (cred)
      {
      printf("might have worked.\n");
      }
    else
      printf("fail.\n");

    }
  }

jni_exception_handle();
}



/********************************************/
/* attempt to use markus' slcs login method */
/********************************************/

/*
org.vpac.grisu.frontend.control.login.LoginParams
LoginParams(String serviceInterfaceUrl, String myProxyUsername, char[] myProxyPassphrase) 

public org.vpac.grisu.frontend.control.login.LoginParams(java.lang.String, java.lang.String, char[]);
  Signature: (Ljava/lang/String;Ljava/lang/String;[C)V
*/

jobject jni_login3_init(void)
{
jmethodID method;
jclass class;
jobject o=NULL;
jstring si, u;
jcharArray p;

if (jni_method_init("org/vpac/grisu/frontend/control/login/LoginParams", "<init>",
                    "(Ljava/lang/String;Ljava/lang/String;[C)V", &class, &method))
  {
  si = (*env)->NewStringUTF(env, "https://grisu.vpac.org/grisu-ws/services/grisu");

  u = (*env)->NewStringUTF(env, "newguy");
  p = jni_jcharArray_from_char("qetuo135");

printf("init 1\n");

  o = (*env)->NewObject(env, class, method, si, u, p);

if (o)
  printf("init 2\n");
  }

jni_exception_handle();
return(o);
}


/*
org.vpac.grisu.frontend.control.login.SlcsLoginWrapper
static GSSCredential	slcsMyProxyInit(String username, char[] password, String idp, LoginParams params) 
public static org.ietf.jgss.GSSCredential slcsMyProxyInit(java.lang.String, char[], java.lang.String, org.vpac.grisu.frontend.control.login.LoginParams)   throws java.lang.Exception;
  Signature: (Ljava/lang/String;[CLjava/lang/String;Lorg/vpac/grisu/frontend/control/login/LoginParams;)Lorg/ietf/jgss/GSSCredential;
*/


void jni_login3(const gchar *idp, const gchar *username, const gchar *password)
{
jmethodID method, m;
jclass class, c;
jobject o, cred, lp;
jstring i, u;
jcharArray p;

// method 2 (instance slcsLoginWrapper and then invoke slcsMyProxyInit)
if (jni_method_init("org/vpac/grisu/frontend/control/login/SlcsLoginWrapper", "<init>",
                    "()V", &class, &method))
  {
printf("step 1\n");

  o = (*env)->NewObject(env, class, method);
  }


if (o)
  {
printf("step 2\n");

  c = (*env)->GetObjectClass(env, o);

  if (c)
    m = (*env)->GetStaticMethodID(env, c, "slcsMyProxyInit", "(Ljava/lang/String;[CLjava/lang/String;Lorg/vpac/grisu/frontend/control/login/LoginParams;)Lorg/ietf/jgss/GSSCredential;");
  else
    g_assert_not_reached();

  if (m)
    {
printf("step 3\n");

    i = (*env)->NewStringUTF(env, idp);
    u = (*env)->NewStringUTF(env, username);
    p = jni_jcharArray_from_char(password);
    lp = jni_login3_init();

// CURRENT - I think this is failing because of a misconfiguration in Markus' .jar
// specifically shibboleth.py is unable to get server certs I think
    cred = (*env)->CallStaticObjectMethod(env, c, m, u, p, i, lp);
    }



  }

jni_exception_handle();
}


/*
org.vpac.grisu.control.ServiceInterface;
org.vpac.grisu.frontend.control.login.LoginManager;

public static org.vpac.grisu.control.ServiceInterface loginCommandline()   throws org.vpac.grisu.frontend.control.login.LoginException;
  Signature: ()Lorg/vpac/grisu/control/ServiceInterface;

java stack trace if above used - nosuchfield error
*/

/*
public static ServiceInterface loginCommandlineShibboleth(String url)
*/

/*
public class au.org.arcs.jcommons.configuration.CommonArcsProperties

public static final java.lang.String ARCS_PROPERTIES_FILE;
  Signature: Ljava/lang/String;


public static au.org.arcs.jcommons.configuration.CommonArcsProperties getDefault();
  Signature: ()Lau/org/arcs/jcommons/configuration/CommonArcsProperties;


public java.lang.String getLastShibIdp();
  Signature: ()Ljava/lang/String;

*/


void jni_login4(void)
{
jmethodID method, mid;
jclass class, cid;
jobject lm, si;
jfieldID field=0;
jstring string;
const char *str;

/**************************************************************/
/* example of how to read the value of a (static) class field */
/**************************************************************/
/*
class = (*env)->FindClass(env, "au/org/arcs/jcommons/configuration/CommonArcsProperties");
// Look for the instance field s in class
if (class)
  field = (*env)->GetStaticFieldID(env, class, "ARCS_PROPERTIES_FILE", "Ljava/lang/String;");

if (field)
  {
  string = (*env)->GetStaticObjectField(env, class, field);
  str = (*env)->GetStringUTFChars(env, string, NULL);
  printf("ARCS_PROPERTIES_FILE = %s\n", str);
  (*env)->ReleaseStringUTFChars(env, string, str);
  }
*/


/* CURRENT - this seems to fail when trying to init the environment */
/* possibly at the arcs common properties setup step ... try to look at this further */

/* instance a loginmanager object */

if (jni_method_init("org/vpac/grisu/frontend/control/login/LoginManager", "<init>", "()V", &class, &method))
  {
printf("step 1\n");

  lm = (*env)->CallObjectMethod(env, class, method);

  if (lm)
    {
    printf("step 2\n");

//ServiceInterface loginCommandlineShibboleth(String url)

//  cid = (*env)->GetObjectClass(env, lm);
//  if (cid)
    {
    printf("step 3\n");

    mid = (*env)->GetStaticMethodID(env, class, "loginCommandlineShibboleth", "(Ljava/lang/String;)Lorg/vpac/grisu/control/ServiceInterface;");

    if (mid)
      {
      printf("step 4\n");

      string = (*env)->NewStringUTF(env, "https://grisu.vpac.org/grisu-ws/services/grisu");

 //     si = (*env)->CallObjectMethod(env, class, mid, string);
      si = (*env)->CallObjectMethod(env, class, mid, NULL);

      if (si)
        printf("step 4\n");


      }

    }





    }


  }




jni_exception_handle();
}


/*
SLCSClient client = new SLCSClient();
GSSCredential gssCredential = client.slcsLogin();

List<IDP> idps = client.getAvailableIDPs();

GSSCredential cred = client.slcsLogin(idps.get(4), username, password);

If you also want to save SLCS certificate to MyProxy?, you should use:

public GSSCredential slcsLogin(IDP idp, String username, String password,
           String myproxyUsername, String myproxyPassword)

*/

/*
public java.util.List getAvailableIDPs()   throws java.io.IOException, java.security.GeneralSecurityException;
  Signature: ()Ljava/util/List;
*/

/****************************************************************/
/* convert an array of IDP objects to something more reasonable */
/****************************************************************/
GSList *jni_slist_from_jlist(jobject jobj)
{
GSList *list=NULL;
jclass c, cid_idp;
jmethodID mid_size, mid_get, mid_idp_getname;
jint i, r;
jstring string;
const gchar *tmp;
jobject idp;

if (!jobj)
  return(NULL);

c = (*env)->GetObjectClass(env, jobj);
if (c)
  {
  mid_size = (*env)->GetMethodID(env, c, "size", "()I");
  mid_get = (*env)->GetMethodID(env, c, "get", "(I)Ljava/lang/Object;");

if (mid_get)
  printf("found get method.\n");
else
  printf("where is get?\n");

  if (mid_size)
    {
    r = (jint) (*env)->CallObjectMethod(env, jobj, mid_size);
    if (r)
      {
      printf("possible result.\n");
      for (i=0 ; i<r ; i++)
        {
        idp = (*env)->CallObjectMethod(env, jobj, mid_get, i);

  cid_idp = (*env)->GetObjectClass(env, idp);
  if (cid_idp)
    mid_idp_getname = (*env)->GetMethodID(env, cid_idp, "getName", "()Ljava/lang/String;");

  if (mid_idp_getname)
    {
    string = (*env)->CallObjectMethod(env, idp, mid_idp_getname);

tmp = (*env)->GetStringUTFChars(env, string, 0);  

//printf("[%d] %s\n", (int) i, tmp);

if (g_strncasecmp(tmp, "ivec", 4) == 0)
  {
printf(" >>> [%d] %s\n", (int) i, tmp);

jni_login(idp, "sean", "xxxxxxx");
//jni_slcs_login2(idp, "sean", "xxxxxx");

//jni_myproxy_upload();
  }


list = g_slist_prepend(list, g_strdup(tmp));

(*env)->ReleaseStringUTFChars(env, string, tmp);  
    }


        }

      }
    }
  }

/*
class = (*env)->FindClass(env, "java/util/List");

if ((*env)->IsInstanceOf(env, jobj, class) == JNI_TRUE)
  printf("True\n");
else
  printf("False\n");
*/

/*
if (class)
  {
printf("found List class\n");

// TODO - invoke methods on obj to extract what we need
//  method = (*env)->GetMethodID(env, jobj, "size", "()I");


  method = (*env)->GetMethodID(env, class, "size", "()I");



  if (method)
printf("found size method\n");


  }
else
  printf("Failed to process list\n");
*/


return(list);
}

/**********************/
/* get a list of idps */
/**********************/
void jni_test(void)
{
const gchar *class_name = "au/org/mams/slcs/client/SLCSClient";
const gchar *method_name = "<init>";
const gchar *method_signature = "()V";
jclass class, c;
jmethodID method, m;
jobject r;

// instance a slcsclient object
if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {

//printf("step1\n");

object_slcsclient = (*env)->NewObject(env, class, method);

  if (object_slcsclient)
    {
//printf("step2\n");

    c = (*env)->GetObjectClass(env, object_slcsclient);
    m = (*env)->GetMethodID(env, c, "getAvailableIDPs", "()Ljava/util/List;");

    if (m)
      {
 //     printf("step3\n");

      r = (*env)->CallObjectMethod(env, object_slcsclient, m);

      if (r)
        {
  //      printf("step4\n");

jni_slist_from_jlist(r);

        }
      }
    }

jni_exception_handle();
  }

}

/*
public static au.org.mams.slcs.client.SLCSConfig getInstance();
  Signature: ()Lau/org/mams/slcs/client/SLCSConfig;
*/

/****************************/
/* check the config read in */
/****************************/
void jni_slcs_config(void)
{
const gchar *class_name = "au/org/mams/slcs/client/SLCSConfig";
const gchar *method_name = "getInstance";
const gchar *method_signature = "()Lau/org/mams/slcs/client/SLCSConfig;";
jclass class, c;
jmethodID method, m;
jobject object, o;

printf("step 1\n");

// instance a slcsclient object
if (jni_method_init(class_name, method_name, method_signature, &class, &method))
  {
  object = (*env)->CallStaticObjectMethod(env, class, method);

  if (object)
    {
    printf("got SLCS config\n");

    c = (*env)->GetObjectClass(env, object);
    m=NULL;
    if (c)
      m = (*env)->GetMethodID(env, c, "getCredentialStore", "()Ljava/lang/String;");
    else
      printf("gah 1\n");

    o = NULL;
    if (m)
      o = (*env)->CallObjectMethod(env, object, m);
    else
      printf("gah 2\n");

    if (o)
      printf("Got credstore name\n");
    else
      printf("gah 3\n");

    }

  }

jni_exception_handle();

}


/********************/
/* start up the jvm */
/********************/
gint jni_init(void)
{
int ret;
JavaVMInitArgs vm_args;
JavaVMOption options[5]; 
gchar *lib;
GSList *item, *list;
GString *cp;

printf("Attempting to fire up JVM...\n");

/* build classpath off ./lib dir */
/* NB: for the SLCS client to find its slcs-client.properties file, the directory */
/* where is resides (./lib) must be in the classpath */
cp = g_string_new("./lib:");
lib = g_build_filename(sysenv.gdis_path, "lib", NULL);
list = file_dir_list_pattern(lib, "*.jar");
item=NULL;
do
  {
  if (item)
    {
    g_string_append_printf(cp, ":./lib/%s", (gchar *) item->data);
    }
  else
    {
// 1st occurance separator exception
    item = list;
    g_string_append_printf(cp, ":./lib/%s", (gchar *) item->data);
    }
  item = g_slist_next(item);
  }
while (item);

free_slist(list);

//printf("cp = [%s]\n", cp->str);

// DEBUG - if more info needed
//options[0].optionString = g_strdup("-verbose:class");
//options[1].optionString = g_strdup("-verbose:jni");
//options[1].optionString = "-Djava.compiler=NONE";  
//options[2].optionString = "-Djava.library.path=.";  

options[0].optionString = g_strdup_printf("-Djava.class.path=%s", cp->str);  
vm_args.nOptions = 1;
vm_args.options = options;
vm_args.version = JNI_VERSION_1_6;
vm_args.ignoreUnrecognized = JNI_TRUE;

ret = JNI_CreateJavaVM(&jvm, (void **) &env, &vm_args);
if(ret < 0)
   printf("\nUnable to Launch JVM\n");       
else
  printf("Success\n");

g_string_free(cp, TRUE);
g_free(options[0].optionString);


/* init */
/*
jni_setServiceInterfaceUrl("https://grisu.vpac.org/grisu-ws/services/grisu");
jni_setMyProxyUsername("someguy");
jni_setMyProxyPassword("xxxxxxx");
jni_setMyProxyServer("myproxy.arcs.org.au");
jni_setMyProxyPort("7512");
*/

/* check init */
/*
jni_getServiceInterfaceUrl();
jni_getMyProxyUsername();
//jni_getMyProxyPassword();
jni_getMyProxyServer();
jni_getMyProxyPort();
*/


//jni_slcs_login("sean", "bollocks", "iVEC");
//jni_idp_test();


/* CURRENT */
/*
jni_slcs_config();
jni_slcs_init();
*/

//jni_login4();

//jni_test();

jni_login3("iVEC", "sean", "bollocks");


myproxy_info("/tmp/x509up_u1000");


jni_exception_handle();

return(0);
}

/*************************/
/* shut down the java vm */
/*************************/
void jni_free(void)
{
if (!jvm)
  return; 

printf("Shutting down JVM...\n");

(*jvm)->DestroyJavaVM(jvm);
}


/* examples */

/*
	key = (*env)->NewStringUTF(env, BLUECOVE_SYSTEM_PROP_NATIVE_LIBRARY_VERSION);
	prop = (*env)->NewStringUTF(env, NATIVE_VERSION);
	oldProp = (*env)->CallObjectMethod(env, s_systemProperties, setPropMethod, key, prop);
*/



/*
JNIEXPORT jstring JNICALL Java_JavaJNI_write(JNIEnv *env,   
jobject thisobject, jstring js){  
const char *temp;  
jstring result = NULL;  
temp = (*env)->GetStringUTFChars(env, js, 0);  
  
printf("Input: %s", temp);  
  
result = (*env)->NewStringUTF(env,   
(const char*) "\nResult: http://www.think-techie.com");  
(*env)->ReleaseStringUTFChars(env, js, temp);  
return result;  
}  
*/


/*
env->GetStringUTFChars(js, 0);  
env->NewStringUTF((const char*) temp);  
env->ReleaseStringUTFChars(js, temp);  
*/


/*
 whenever a JNI function fails, you can use something like this to
 see the reason:

 if (env->ExceptionOccurred()) {
   env->ExceptionDescribe();
 }

// have to clear the exception before JNI will work again.
 env->ExceptionClear();
*/


