#include "credentials.h"
#include "mdb.h"
#include <asprintf/asprintf.h>
#include <curl/curl.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <b64/b64.h>

typedef struct upload_status
{
  size_t bytes_read;
} UploadStatus;

typedef struct curl_slist CurlSList;

char* payload_text;

// char FROM[]         = "knock knock...<knock@myopendoor.de>";
char FROM[]         = "<kohlhaas.michael@mein.gmx>";
// char MESSAGE_ID[]   = "<dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>" ;
char SUBJECT[]      = "Verlängerung der epidemischen Lage von nationaler Tragweite";
char SALUTATION[]   = "Guten Tag";
char EMAIL_DOMAIN[] = "bundestag.de";
// char EMAIL_DOMAIN[] = "chammy.info";
char BODY[]         = 
",\r\n\r\n\
am 24. November endet die Wirkung der epidemischen Lage von nationaler Tragweite.\r\n\r\n\
Meine Frage an Sie: Werden Sie für eine Verlängerung der epidemischen Lage stimmen?\r\n\r\n\
Viele Grüße,\r\n\
M. Kohlhaas";
