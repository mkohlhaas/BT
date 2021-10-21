#include "smtp-mail.h"

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

void
send_mail(char* personal_salut, char* email)
{
  // size_t output_length;
  // char*  encoded = base64_encode((const unsigned char*)payload_text, strlen(payload_text), &output_length);
  // printf("%s\n", encoded);

  CURL*        curl;
  CURLcode     error      = CURLE_OK;
  CurlSList*   recipients = NULL;
  UploadStatus upload_ctx = { 0 };

  asprintf(&payload_text, "From: %s\r\nTo: %s\r\nMessage-ID: %s\r\nContent-Type: text/plain; charset=utf-8\r\nSubject: =?utf-8?B?%s?=\r\n\r\n%s %s%s",
		           FROM, email, MESSAGE_ID,
			   b64_encode((const unsigned char*)SUBJECT, strlen((const char*)SUBJECT)),
			   SALUTATION, personal_salut, BODY);
  printf("%s\n", payload_text);

  curl = curl_easy_init();
  if (curl) {
    recipients = curl_slist_append(recipients, email);
    curl_easy_setopt(curl, CURLOPT_USERNAME    , USER);
    curl_easy_setopt(curl, CURLOPT_PASSWORD    , PASSWD);
    curl_easy_setopt(curl, CURLOPT_URL         , SMPT_SERVER);
    curl_easy_setopt(curl, CURLOPT_USE_SSL     , CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM   , FROM);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT   , recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA    , &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD      , 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE     , 1L);

    error = curl_easy_perform(curl); // Send the message
    if (error) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(error));
      exit(error);
    }
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
  }
  free(payload_text);
}

int
main()
{
  char* email;
printf("%zu\n", sizeof mdbs / sizeof *mdbs);

  for (size_t i = 0; i < sizeof mdbs / sizeof *mdbs; i++) {
    asprintf(&email, "%s@%s", mdbs[i].email, EMAIL_DOMAIN);
    send_mail(mdbs[i].salutation, email);
    free(email);
  }
}
