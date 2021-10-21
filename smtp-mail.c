#define _GNU_SOURCE
#include "credentials.h"
#include "mdb.h"
#include <curl/curl.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void
get_date(char buffer[static 256])
{
   time_t     now        = time(NULL);
   struct tm* time       = localtime(&now);
   strftime(buffer, 256, "Date: %c\r\n", time);
}

static int   mod_table[]      = { 0, 2, 1 };
static char  encoding_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

char*
base64_encode(const unsigned char* data, size_t input_length, size_t* output_length)
{
  *output_length     = 4 * ((input_length + 2) / 3);
  char* encoded_data = malloc(*output_length);
  if (encoded_data == NULL) return NULL;
  for (int i = 0, j = 0; i < input_length;) {
    uint32_t octet_a  = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b  = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c  = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t triple   = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }
  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[*output_length - 1 - i] = '=';
  return encoded_data;
}

char TO[]         = "To: DressGuard<dressguard@chammy.info>\r\n";
char FROM[]       = "From: knock knock ...<knock@myopendoor.de>\r\n";
char MESSAGE_ID[] = "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>\r\n" ;
char SUBJECT[]    = "Subject: epidemische Lage nationaler Tragweite - Verlängerung\r\n";
char BODY[]       = "The body of the message starts here.\r\n\r\nIt could be a lot of lines, could be MIME encoded, whatever.\r\nCheck RFC5322.\r\n";;

char* payload_text;

typedef struct upload_status
{
  size_t bytes_read;
} UploadStatus;

static size_t
payload_source(char* ptr, size_t size, size_t nmemb, void* userp)
{
  const char*   data;
  UploadStatus* upload_ctx = (UploadStatus*)userp;
  size_t        room       = size * nmemb;

  if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) return 0;

  data = &payload_text[upload_ctx->bytes_read];

  if (data) {
    size_t len = strlen(data);
    if (room < len) len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;
    return len;
  }
  return 0;
}

int
main()
{
  typedef struct curl_slist CurlSList;

  asprintf(&payload_text, "%s%s%s%s\r\n%s", TO, FROM, MESSAGE_ID, SUBJECT, BODY);

  size_t output_length;
  char*  encoded = base64_encode((const unsigned char*)payload_text, strlen(payload_text), &output_length);
  printf("%s\n", encoded);

  CURL*        curl;
  CURLcode     error      = CURLE_OK;
  CurlSList*   recipients = NULL;
  UploadStatus upload_ctx = { 0 };

  curl = curl_easy_init();
  if (curl) {
    recipients = curl_slist_append(recipients, TO_ADDR);
    curl_easy_setopt(curl, CURLOPT_USERNAME    , USER);
    curl_easy_setopt(curl, CURLOPT_PASSWORD    , PASSWD);
    curl_easy_setopt(curl, CURLOPT_URL         , SMPT_SERVER);
    curl_easy_setopt(curl, CURLOPT_USE_SSL     , (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM   , FROM_ADDR);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT   , recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA    , &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD      , 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE      , 1L);
    // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);

    error = curl_easy_perform(curl); // Send the message
    if (error) fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(error));

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  }
  return (int)error;
}
