/* xor.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_hex(const char *s, const char *text, size_t len) {
	printf("%s (hex): ", text);
	for(size_t i = 0; i < len; i++) {
		printf("%02X ", (unsigned char)s[i]);
	}
	printf("(len=%d)\n", len);
}

void xor_strings(const char *cleartext, const char *key, char *ciphertext, size_t cleartext_length) {
	size_t key_length = strlen(key);
	for(size_t i = 0; i < cleartext_length; i++) {
		ciphertext[i] = cleartext[i] ^ key[i % key_length];
	}
}

int main(char argc, char **argv) {
	if (argc < 3) {
		printf("Usage: ./%s <cleartext> <key>", argv[0]);
		return -1;
	}

	const char *cleartext = argv[1];
	const char *key = argv[2];
	size_t cleartext_length = strlen(cleartext);

	print_hex(cleartext, "cleartext", cleartext_length);
	print_hex(key, "key", strlen(key));
	
	char *ciphertext = malloc(++cleartext_length);
	if (!ciphertext) {
		perror("malloc");
		return -1;
	}

	xor_strings(cleartext, key, ciphertext, cleartext_length);
	print_hex(ciphertext, "ciphertext", cleartext_length);

	// Decrypt to verify result
	char *new_cleartext = malloc(cleartext_length);
	xor_strings(ciphertext, key, new_cleartext, cleartext_length);
	printf("New cleartext: '%.*s'\n", (int) cleartext_length, new_cleartext);

	free(ciphertext);
	free(new_cleartext);
	return 0;
}
