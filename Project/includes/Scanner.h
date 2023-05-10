#ifndef SCANNER_H
#define SCANNER_H

enum ScannerErrorCode {
    SCANNER_ERROR_NONE,
    SCANNER_ERROR_UNRECOGNIZED_TOKEN,
    SCANNER_ERROR_UNEXPECTED_TOKEN,
    SCANNER_ERROR_FAILED_ALLOCATION
};

typedef struct ScannerError {
    enum ScannerErrorCode code;
    Position pos;
    char c;
} ScannerError;

ScannerError Scanner_scan(const char *input, TokenList **ret);

#endif
