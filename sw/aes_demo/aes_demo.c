#define __riscv__

#include <stdint.h>
#include <gpio.h>
#include <uart.h>
#include <string_lib.h>
#include <pulpino.h>
#include "aes.h"

#define AES_TIMEOUT 100000

// Nexys A7: LED[15:0] = gpio_out[31:16]
#define LED_PASS    16
#define LED_FAIL    17

// Keys and blocks are stored most significant word first, as written in the standards.
typedef struct {
  const char *name;
  uint32_t    key[8];
  uint32_t    plaintext[4];
  uint32_t    ciphertext[4];
} aes_vector_t;

static const aes_vector_t vectors[] = {
  {
    "FIPS-197 C.3 AES-256",
    { 0x00010203, 0x04050607, 0x08090a0b, 0x0c0d0e0f,
      0x10111213, 0x14151617, 0x18191a1b, 0x1c1d1e1f },
    { 0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff },
    { 0x8ea2b7ca, 0x516745bf, 0xeafc4990, 0x4b496089 }
  },
  {
    "SP 800-38A ECB-AES256 block 1",
    { 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781,
      0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4 },
    { 0x6bc1bee2, 0x2e409f96, 0xe93d7e11, 0x7393172a },
    { 0xf3eed1bd, 0xb5d2a03c, 0x064b5a7e, 0x3db181f8 }
  },
  {
    "SP 800-38A ECB-AES256 block 2",
    { 0x603deb10, 0x15ca71be, 0x2b73aef0, 0x857d7781,
      0x1f352c07, 0x3b6108d7, 0x2d9810a3, 0x0914dff4 },
    { 0xae2d8a57, 0x1e03ac9c, 0x9eb76fac, 0x45af8e51 },
    { 0x591ccb10, 0xd410ed26, 0xdc5ba74a, 0x31362870 }
  }
};

#define NUM_VECTORS (sizeof(vectors) / sizeof(vectors[0]))

// STATUS.ready drops a few cycles after a command is written, so wait for 0 first, then 1.
static int aes_wait(void)
{
  int t;

  for (t = AES_TIMEOUT; AES_STATUS & AES_STATUS_READY; t--)
    if (t == 0)
      return -1;

  for (t = AES_TIMEOUT; !(AES_STATUS & AES_STATUS_READY); t--)
    if (t == 0)
      return -1;

  return 0;
}

static int aes_set_key(const uint32_t key[8], int key256)
{
  for (int i = 0; i < 8; i++)
    AES_KEY(i) = key[i];

  AES_CONFIG = key256 ? AES_CONFIG_KEY256 : 0;
  AES_CTRL   = AES_CTRL_INIT;

  return aes_wait();
}

// RESULT follows the datapath selected by CONFIG, so CONFIG must not change before it is read.
static int aes_crypt(const uint32_t in[4], uint32_t out[4], int encrypt, int key256)
{
  AES_CONFIG = (key256 ? AES_CONFIG_KEY256 : 0) | (encrypt ? AES_CONFIG_ENCRYPT : 0);

  for (int i = 0; i < 4; i++)
    AES_BLOCK(i) = in[i];

  AES_CTRL = AES_CTRL_NEXT;

  if (aes_wait())
    return -1;

  for (int i = 0; i < 4; i++)
    out[i] = AES_RESULT(i);

  return 0;
}

static int blocks_equal(const uint32_t a[4], const uint32_t b[4])
{
  for (int i = 0; i < 4; i++)
    if (a[i] != b[i])
      return 0;
  return 1;
}

static void print_block(const uint32_t b[4])
{
  printf("%08x%08x%08x%08x", b[0], b[1], b[2], b[3]);
}

static void print_ascii(uint32_t w)
{
  printf("%c%c%c%c", (w >> 24) & 0xff, (w >> 16) & 0xff, (w >> 8) & 0xff, w & 0xff);
}

// Returns 0 if the operation result matches the expected block.
static int check(const char *what, int status, const uint32_t got[4], const uint32_t expected[4])
{
  printf("  %s: ", what);

  if (status) {
    printf("TIMEOUT\n");
    return 1;
  }

  print_block(got);

  if (blocks_equal(got, expected)) {
    printf(" OK\n");
    return 0;
  }

  printf(" FAIL\n    expected: ");
  print_block(expected);
  printf("\n");
  return 1;
}

int main()
{
  int      errors = 0;
  uint32_t ciphertext[4];
  uint32_t plaintext[4];

  uart_set_cfg(0, 325); // 9600 baud, no parity (50 MHz CPU)

  set_pin_function(LED_PASS, FUNC_GPIO);
  set_pin_function(LED_FAIL, FUNC_GPIO);
  set_gpio_pin_direction(LED_PASS, DIR_OUT);
  set_gpio_pin_direction(LED_FAIL, DIR_OUT);
  set_gpio_pin_value(LED_PASS, 0);
  set_gpio_pin_value(LED_FAIL, 0);

  printf("\nAES accelerator demo\ncore: \"");
  print_ascii(AES_NAME0);
  print_ascii(AES_NAME1);
  printf("\" version ");
  print_ascii(AES_VERSION);
  printf("\n");

  for (unsigned int n = 0; n < NUM_VECTORS; n++) {
    const aes_vector_t *v = &vectors[n];

    printf("\n%s\n", v->name);

    if (aes_set_key(v->key, 1)) {
      printf("  key init: TIMEOUT\n");
      errors++;
      continue;
    }

    errors += check("encrypt", aes_crypt(v->plaintext, ciphertext, 1, 1), ciphertext, v->ciphertext);
    errors += check("decrypt", aes_crypt(v->ciphertext, plaintext, 0, 1), plaintext, v->plaintext);
  }

  if (errors == 0) {
    printf("\nPASS\n");
    set_gpio_pin_value(LED_PASS, 1);
  } else {
    printf("\nFAIL: %d error(s)\n", errors);
    set_gpio_pin_value(LED_FAIL, 1);
  }

  uart_wait_tx_done();

  while (1) {}
}
