// Fill in  your WiFi networks SSID and password
#define SECRET_SSID "FiberHGW_ZTKNC2_2.4GHz"
#define SECRET_PASS "T3ehzE7T494x"

// Fill in the hostname of your AWS IoT broker
#define SECRET_BROKER "ac40od4fxn5tc-ats.iot.eu-central-1.amazonaws.com"
#define SECRET_PORT 8883

// Fill in the boards public certificate
extern char SECRET_CERTIFICATE[2048];

const char TEST_CERTIFICATE[] = R"(
-----BEGIN CERTIFICATE-----
MIICzjCCAbagAwIBAgIUHVrf3qhLM0a3sOqraMGu97r1nRcwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI0MDMwNDA3NTIy
OVoXDTQ5MTIzMTIzNTk1OVowXjELMAkGA1UEBhMCQVUxDTALBgNVBAgTBHZJRU4x
DTALBgNVBAcTBFZpZW4xDDAKBgNVBAoTA1ImQzEOMAwGA1UECxMFRW1iZWQxEzAR
BgNVBAMTClBvcnRlbnRhMDMwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATvUSis
A5v1AbVQvEEtxwVElJGfVP+Ccwgsx6ti/YnoSmZIn+aaxnAIIZFLerGJ9mD6el7e
a0cgtAIVpx/4vOK9o2AwXjAfBgNVHSMEGDAWgBT6ZyHWxDZBBI6HAy/sqo2UHK++
6jAdBgNVHQ4EFgQU/c9x+wI2NJePSzw7DNYCJsD3+kIwDAYDVR0TAQH/BAIwADAO
BgNVHQ8BAf8EBAMCB4AwDQYJKoZIhvcNAQELBQADggEBAKOCx3u+tRxgbYVlcJFP
5ZYUhyaQONiqFAjNMmyI++ydHUzWCHqGePjmPfLhRybUP6kKKnDNV7KoNM8hQE/Y
A93SCUHWTipPUQffab8EA4XQTdVj0jxtrE/GgJMYB9X8HWeJMehbYQnGM5hPc9I1
Ni99Gbr9CzSTZVtl+t98+sMZK99cnt37BV+HpZD3qPIDfHBbyHrVKwDw0I9v2it1
3c7hgshKmAnOhYvDyEPZoCxUus1mTPtlwKFJLEcJfge17EVys7i43QLJSLOMKC0t
S+hUtHwbdZhY8uOXDTb/OTkDqpfuEn6/BrtXzouuBCR9NFA8hXAssQuR8boyDacz
K3w=
-----END CERTIFICATE-----
)";


struct wifiCredentials{
    char ssid[128];
    char pass[128];
};
