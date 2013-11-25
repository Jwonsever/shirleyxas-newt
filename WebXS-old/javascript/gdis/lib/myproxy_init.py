#
# myproxy client
#
# Tom Uram <turam@mcs.anl.gov>
# 2005/08/04
#


import os
import socket
from OpenSSL import crypto, SSL

#from M2Crypto import X509
#import M2Crypto
#from M2Crypto import m2

from arcs.gsi import CertificateRequest, Certificate, ProxyCertificate




class GetException(Exception): pass
class RetrieveProxyException(Exception): pass


debug = 2
def debuglevel(level):
    global debug
    return debug >= level


def create_cert_req():
    return


 
    
def deserialize_response(msg):
    """
    Deserialize a MyProxy server response
    
    Returns: integer response, errortext (if any)
    """
    
    lines = msg.split('\n')
    
    # get response value
    responselines = filter( lambda x: x.startswith('RESPONSE'), lines)
    responseline = responselines[0]
    response = int(responseline.split('=')[1])
    
    # get error text
    errortext = ""
    errorlines = filter( lambda x: x.startswith('ERROR'), lines)
    for e in errorlines:
        etext = e.split('=')[1]
        errortext += etext
    
    return response,errortext
 
               
def deserialize_certs(inp_dat):
    
    pem_certs = []
    
    dat = inp_dat
    
    import base64
    while dat:

        # find start of cert, get length        
        ind = dat.find('\x30\x82')
        if ind < 0:
            break
            
        len = 256*ord(dat[ind+2]) + ord(dat[ind+3])

        # extract der-format cert, and convert to pem
        c = dat[ind:ind+len+4]
        x509 = crypto.load_certificate(crypto.FILETYPE_ASN1,c)
        pem_cert = crypto.dump_certificate(crypto.FILETYPE_PEM,x509)
        pem_certs.append(pem_cert)

        # trim cert from data
        dat = dat[ind + len + 4:]
    

    return pem_certs


CMD_GET="""VERSION=MYPROXYv2
COMMAND=0
USERNAME=%s
PASSPHRASE=%s
LIFETIME=%d\0"""

CMD_PUT="""VERSION=MYPROXYv2
COMMAND=1
USERNAME=%s
PASSPHRASE=%s
RETRIEVER=*
LIFETIME=%d"""

def my_callback(conn, x590, a, b, c):
    return True

def myproxy_init_py(hostname,username,passphrase,outfile,lifetime=43200,port=7512):
    """
    Function to retrieve a proxy credential from a MyProxy server
    
    Exceptions:  GetException, RetrieveProxyException
    """
    
#    import socket, ssl, pprint
#    s = socket()
#    c = ssl.wrap_socket(s, cert_reqs=ssl.CERT_REQUIRED, ssl_version=ssl.PROTOCOL_SSLv3,
#                        ca_certs='/home/sean/.globus/certificates/1e12d831.0')
#    c.connect((hostname, port))

# naive and incomplete check to see if cert matches host
#    cert = c.getpeercert()
#
#    if not cert:
#      print "server auth failed."
    from myproxy import client

    c = client.MyProxyClient(
        hostname='myproxy2.arcs.org.au', 
        serverDN='/C=AU/O=APACGrid/OU=VPAC/CN=myproxy2.arcs.org.au')

    c.put(username, 
          passphrase, 
          '/home/sean/.globus-slcs/usercert.pem', 
          '/home/sean/.globus-slcs/userkey.pem', 
          lambda *x: 'xxx', 
          retrievers='*')

    return


    hostname = 'myproxy.arcs.org.au'


    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# require a certificate from the server
    ssl_sock = ssl.wrap_socket(s, cert_reqs=ssl.CERT_REQUIRED, ssl_version=ssl.PROTOCOL_SSLv3,
                        ca_certs='/home/sean/.globus/certificates/1e12d831.0')
    ssl_sock.connect((hostname, port))


    print repr(ssl_sock.getpeername())
    print ssl_sock.cipher()
    print pprint.pformat(ssl_sock.getpeercert())



    # must be v3 for myproxy
    context = SSL.Context(SSL.SSLv3_METHOD)
    
    #context.load_verify_locations(None, "/home/sean/.globus-slcs")

# --- SDF
    #if self.caCertFilePath or self.caCertDir:
    #        context.load_verify_locations(cafile=self.caCertFilePath,
    #                                      capath=self.caCertDir)
                            
            # Stop if peer's certificate can't be verified
    #        context.set_allow_unknown_ca(False)
    #    else:
    #        context.set_allow_unknown_ca(True)
            
    #try:
    #    context.load_cert_chain("/home/sean/.globus-slcs/usercert.pem",
    #                            "/home/sean/.globus-slcs/userkey.pem",
    #                            lambda *x: 'xxx')
    #except Exception, e:
    #     print "failed to process certs for SSL connection."
            


# --- SDF
# --- TODO - write a local proxy file and use to auth to myproxy server
    print "TODO - create local proxy: " + outfile

#    mykey = crypto.load_privatekey(crypto.FILETYPE_PEM,open('/home/sean/.globus-slcs/userkey.pem').read(),'xxx')
#    mycert = crypto.load_certificate(crypto.FILETYPE_PEM,open('/home/sean/.globus-slcs/usercert.pem').read())

#    proxy = ProxyCertificate(mycert, mykey)

#    lp = CertificateRequest()

#    lp.set_dn(mycert.get_subject_name()) 

#    dn = mycert.get_subject_name()
#    req = X509.Request()
#    req.set_subject_name(dn)
#    req.sign(pkey, 'sha1')

# server auth
#    context.load_verify_locations(None, "/home/sean/.globus/certificates");

# client auth
#    context.load_cert_chain(ownerCertFile, keyfile=ownerKeyFile, callback=lambda *ar, **kw: ownerPassphrase)

    #context.load_client_ca('/home/sean/.globus-slcs/usercert.pem')
    #context.set_verify(SSL.VERIFY_CLIENT_ONCE | SSL.VERIFY_PEER, my_callback) 

    # disable for compatibility with myproxy server (er, globus)
    # globus doesn't handle this case, apparently, and instead
    # chokes in proxy delegation code
    context.set_options(0x00000800L)
    #context.set_options(m2.SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS)
   # context.set_options(M2Crypto.SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS)
    
    # connect to myproxy server
    print "debug: connect to myproxy server"
    conn = SSL.Connection(context,socket.socket())
    conn.connect((hostname,port))
    
    # send globus compatibility stuff
    print "debug: send globus compat byte"
    conn.write('0')

    # send get command
    print "debug: send put command"
    cmd_get = CMD_PUT % (username,passphrase,lifetime)
    conn.write(cmd_get)

    # process server response
    print "debug: get server response"
    dat = conn.recv(8192)
    print dat
    response,errortext = deserialize_response(dat)
    if response:
        raise GetException(errortext)
    else:
        print "debug: server response ok"

# if it worked - should get a CSR from the server
# The server will generate a public/private key pair and send a NULL-terminated PKCS#10 [RFC2986]
# certificate request to the client. The client will then send a proxy certificate [RFC3820],
# containing the public key from the certificate request, signed by its private key, followed by
# the corresponding certificate chain, back to the server. The client will set the subject of the
# proxy certificate as specified in [RFC3820] Section 3.4, ignoring the subject in the server's
# certificate request. The format of the client's message is: one byte (unsigned binary) encoding
# the number of certificates to be sent (255 maximum), followed by the newly signed proxy certificate,
# followed by the certificates in the chain. Each certificate is DER-encoded. The certificate chain
# MUST include the end-entity certificate signed by a CA, and MAY also include CA certificates.

    print "debug: receiving CSR"
    dat = conn.recv(8192)

#    req = crypto.load_certificate_request(crypto.FILETYPE_ASN1, dat)
#
#    proxy = crypto.load_certificate(crypto.FILETYPE_PEM,open('/tmp/x509up_u1000').read())
#    mykey = crypto.load_privatekey(crypto.FILETYPE_PEM,open('/tmp/x509up_u1000').read())

#    req.sign(mykey, 'sha1')

#    derproxy = crypto.dump_certificate(crypto.FILETYPE_ASN1, proxy)
#    derreq = crypto.dump_certificate_request(crypto.FILETYPE_ASN1, req)
    
#    conn.write(chr(2) + derreq + derproxy)
#    return;

#        certTxt = X509.load_cert(certFile).as_pem()
#        keyTxt = open(keyFile).read()
#        conn.send(certTxt + keyTxt)

#    mykey = crypto.load_privatekey(crypto.FILETYPE_PEM,open('/home/sean/.globus-slcs/userkey.pem').read(),'xx')
#    mycert = crypto.load_certificate(crypto.FILETYPE_PEM,open('/home/sean/.globus-slcs/usercert.pem').read())
#    derdert = crypto.dump_certificate(crypto.FILETYPE_ASN1, mycert)
#    try:
#        print mycert.as_text()
#    except:
#        print "Unexpected error:", sys.exc_info()[0]

# param certFile: user's X.509 certificate in PEM format
# param keyFile: equivalent private key file in PEM format
#    from arcs.gsi import CertificateRequest, Certificate, ProxyCertificate

    req = CertificateRequest(dat)

#    if isinstance(certFile, Certificate):
#            usercert = certFile
#    else:
#            usercert = Certificate(certFile, keyFile,
#                                   callback=passphraseCallback)

    usercert = Certificate('/home/sean/.globus-slcs/usercert.pem',
                           '/home/sean/.globus-slcs/userkey.pem', lambda x: 'xxx') 
    proxy = ProxyCertificate(usercert, proxykey=req.get_pubkey())

    try:
        proxy.sign()
        conn.send(chr(2) + proxy.as_der() + usercert.as_der())
    except:
        print "Unexpected error:", sys.exc_info()



#M2Crypto.X509.load_request_der_string
# x509 request
#    req = crypto.load_certificate_request(crypto.FILETYPE_ASN1, dat)


#
#    req.sign(mykey, 'md5')
#    conn.write(chr(2) + req + dercert)


#TODO - DEBUG - print?
#    print "sub: " + str(req.get_subject)
#crypto.dump_certificate()
#    print req.as_text()

#    test1 = 1

#    if (test1):
#        cert = crypto.X509()
#  Generate a certificate given a certificate request.
#  req        - Certificate reqeust to use
#  issuerCert - The certificate of the issuer
#  issuerKey  - The private key of the issuer
#    cert.set_serial_number(serial)
#    cert.gmtime_adj_notBefore(notBefore)
#    cert.gmtime_adj_notAfter(notAfter)
#        cert.set_issuer(mycert.get_subject())
#        cert.set_subject(mycert.get_subject())
#        cert.set_pubkey(req.get_pubkey())
#        cert.sign(mykey, 'sha1')

#        buff = crypto.dump_certificate(crypto.FILETYPE_ASN1, cert)
#        buff2 = crypto.dump_certificate(crypto.FILETYPE_ASN1, mycert)

#    else:
#        req.sign(mykey, 'md5')
#        buff = crypto.dump_certificate_request(crypto.FILETYPE_ASN1, req)


#    conn.write(chr(2) + buff + buff2)
#    conn.write(chr(2) + cert.as_der() + usercert.as_der())


#    mykey = crypto.load_privatekey(crypto.FILETYPE_PEM,open('/home/sean/.globus-slcs/userkey.pem').read(),'xxxx')
#    mycert = crypto.load_certificate(crypto.FILETYPE_PEM,open('/home/sean/.globus-slcs/usercert.pem').read())
#    req.sign(mykey, 'sha1')
# number of cert(s) to send back
#    conn.write('2')
#    conn.write(req.as_der() + mycert.as_der())


# Send certificate and private key
#        certTxt = X509.load_cert(certFile).as_pem()
#        keyTxt = open(keyFile).read()
#        conn.send(certTxt + keyTxt)


#    req.sign('/home/sean/.globus-slcs/userkey.pem', 'md5')

    #req.as_der()


    #req = X509.Request()
    #req.set_subject_name(name)
    #req.sign(pkey, 'md5')
    #req.as_der(), key.as_pem(cipher=None))


    # generate and send certificate request
    # - The client will generate a public/private key pair and send a 
    #   NULL-terminated PKCS#10 certificate request to the server.
    #print "debug: send cert request"
    #certreq,privatekey = create_cert_req()
    #conn.send(certreq)

    # process certificates
    # - 1 byte , number of certs
    #dat = conn.recv(1)
    #numcerts = ord(dat[0])
    
    # - n certs
    #print "debug: receive certs"
    #dat = conn.recv(8192)

    #print "debug: dumping cert data to myproxy.dump"
    #f = file('myproxy.dump','w')
    #f.write(dat)
    #f.close()

    # process server response
    #print "debug: get server response"
    #resp = conn.recv(8192)
    #response,errortext = deserialize_response(resp)
    #if response:
    #    raise RetrieveProxyException(errortext)
    #else:
    #    print "debug: server response ok"

    # deserialize certs from received cert data
    #pem_certs = deserialize_certs(dat)
    #if len(pem_certs) != numcerts:
    #    print "Warning: %d certs expected, %d received" % (numcerts,len(pem_certs))

    # write certs and private key to file
    # - proxy cert
    # - private key
    # - rest of cert chain

    #print "debug: write proxy and certs to", outfile
    #f = file(outfile,'w')
    #f.write(pem_certs[0])
    #f.write(privatekey)
    #for c in pem_certs[1:]:
    #    f.write(c)
    #f.close()
    
    
myproxy_init = myproxy_init_py
    
    
if __name__ == '__main__':
    import sys
    import optparse
    import getpass
    
    parser = optparse.OptionParser()
    parser.add_option("-s", "--pshost", dest="host", 
                       help="The hostname of the MyProxy server to contact")
    parser.add_option("-p", "--psport", dest="port", default=7512,
                       help="The port of the MyProxy server to contact")
    parser.add_option("-l", "--username", dest="username", 
                       help="The username with which the credential is stored on the MyProxy server")
    parser.add_option("-o", "--out", dest="outfile", 
                       help="The username with which the credential is stored on the MyProxy server")
    parser.add_option("-t", "--proxy-lifetime", dest="lifetime", default=43200,
                       help="The username with which the credential is stored on the MyProxy server")
    parser.add_option("-d", "--debug", dest="debug", default=0,
                       help="Debug mode: 1=print debug info ; 2=print as in (1), and dump data to myproxy.dump")

    (options,args) = parser.parse_args()
    
    debug = options.debug

    # process options
#    host = options.host

    host = "myproxydev.arcs.org.au"
    port = 7512
    username = "seantest7"
    passphrase = "qetuo135"
    lifetime = 1000

#    if not host:
#        print "Error: MyProxy host not specified"
#        sys.exit(1)
#    port = int(options.port)
#    username = options.username
#    if not username:
#        if sys.platform == 'win32':
#            username = os.environ["USERNAME"]
#        else:
#            import pwd
#            username = pwd.getpwuid(os.geteuid())[0]
#    lifetime = int(options.lifetime)
    
    outfile = options.outfile
    if not outfile:
        if sys.platform == 'win32':
            outfile = 'proxy'
        elif sys.platform in ['linux2','darwin']:
            outfile = '/tmp/x509up_u%s' % (os.getuid())

    # Get MyProxy password
    #passphrase = getpass.getpass()
    # Retrieve proxy cert
    #print "Upload: " + outfile

    try:
        ret = myproxy_init(host,username,passphrase,outfile,lifetime=lifetime,port=port)
 #       print "A proxy has been uploaded for user %s" % (username)
    except Exception,e:
        if debuglevel(1):
            import traceback
            traceback.print_exc()
        else:
            print "Error:", e
 
