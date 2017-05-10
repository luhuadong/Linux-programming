#ifndef TelnetAuthH
#define TelnetAuthH


unsigned int GetTelentAuthKey(char *buf, unsigned int length);

unsigned int GetTelentAuthPwd(const char *key, unsigned int keyLen, char *password, unsigned int pwdLen);

unsigned int GetTelenetAuthKeyLen(const char *key, unsigned int length);

int TransBase64Encode(const unsigned char *in, unsigned int ilen, char *out, unsigned int *olen);

int TransBase64Decode(const unsigned char *in, unsigned int ilen, char *out, unsigned int *olen);
#endif
