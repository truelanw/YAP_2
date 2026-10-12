#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LETTERS 10
#define MAX_EXPR 256
#define SEP (-1)
#define END (-2)

typedef struct {
    int expr[MAX_EXPR];
    int expr_len;
    char unic_letters[MAX_LETTERS];
    int letter_count;
    int is_leading[MAX_LETTERS];
    int total_words;
    int word_start[MAX_EXPR];
    int word_len[MAX_EXPR];
    int result_word;
    int max_addend_len;
} Puzzle;

static int used[10];
static int digit[MAX_LETTERS];
static int solution[MAX_LETTERS];

/* Парсер сохраняет структуру выражения и заранее вычисляет длины слов. */
static int parse_puzzle(const char *in, Puzzle *p) {
    memset(p, 0, sizeof(*p));
    char loc_buf[512];
    strncpy(loc_buf, in, sizeof(loc_buf) - 1);
    loc_buf[sizeof(loc_buf) - 1] = '\0';

    char *token = strtok(loc_buf, "=+ \t\r\n");
    while (token) {
        int length = (int)strlen(token);
        if (p->expr_len + length + 1 >= MAX_EXPR ||
            p->total_words >= MAX_EXPR - 1) return 0;

        p->word_start[p->total_words] = p->expr_len;
        p->word_len[p->total_words] = length;

        for (int i = 0; i < length; ++i) {
            char c = token[i];
            int indx = -1;
            for (int j = 0; j < p->letter_count; ++j) {
                if (p->unic_letters[j] == c) {
                    indx = j;
                    break;
                }
            }
            if (indx == -1) {
                if (p->letter_count >= MAX_LETTERS) return 0;
                indx = p->letter_count++;
                p->unic_letters[indx] = c;
            }
            p->expr[p->expr_len++] = indx;
        }

        int first = p->expr[p->word_start[p->total_words]];
        if (length > 1) p->is_leading[first] = 1;

        p->expr[p->expr_len++] = SEP;
        p->total_words++;
        token = strtok(NULL, "=+ \t\r\n");
    }

    if (p->expr_len == 0 || p->total_words < 3 ||
        p->total_words - 1 > 7) return 0;

    p->expr[p->expr_len - 1] = END;
    p->result_word = p->total_words - 1;
    p->max_addend_len = 0;
    for (int k = 0; k < p->result_word; ++k) {
        if (p->word_len[k] > p->max_addend_len)
            p->max_addend_len = p->word_len[k];
    }
    return 1;
}

static void format_answer(const Puzzle *p, const int *digits, char *out) {
    char *o = out;
    for (int k = 0; k < p->total_words; ++k) {
        int start = p->word_start[k], len = p->word_len[k];
        for (int j = 0; j < len; ++j)
            *o++ = (char)('0' + digits[p->expr[start + j]]);

        if (k == p->result_word - 1) {
            memcpy(o, " = ", 3); o += 3;
        } else if (k != p->total_words - 1) {
            memcpy(o, " + ", 3); o += 3;
        }
    }
    *o = '\0';
}

static int solve_column(const Puzzle *p, int col, long long carry);

static int assign_addend_digits(const Puzzle *p, int col, int word,
                                long long carry, long long sum) {
    if (word == p->result_word) {
        int expected = (int)(sum % 10);
        long long next_carry = sum / 10;
        int result_len = p->word_len[p->result_word];

        if (col >= result_len) {
            if (expected != 0) return 0;
            return solve_column(p, col + 1, next_carry);
        }

        int index = p->word_start[p->result_word] + result_len - 1 - col;
        int result_letter = p->expr[index];

        if (digit[result_letter] != -1) {
            if (digit[result_letter] != expected) return 0;
            return solve_column(p, col + 1, next_carry);
        }

        if (used[expected]) return 0;
        if (expected == 0 && p->is_leading[result_letter]) return 0;

        digit[result_letter] = expected;
        used[expected] = 1;

        if (solve_column(p, col + 1, next_carry)) return 1;

        used[expected] = 0;
        digit[result_letter] = -1;
        return 0;
    }

    int len = p->word_len[word];
    if (col >= len)
        return assign_addend_digits(p, col, word + 1, carry, sum);

    int index = p->word_start[word] + len - 1 - col;
    int letter = p->expr[index];

    if (digit[letter] != -1)
        return assign_addend_digits(p, col, word + 1, carry,
                                    sum + digit[letter]);

    for (int d = 0; d <= 9; ++d) {
        if (used[d]) continue;
        if (d == 0 && p->is_leading[letter]) continue;

        digit[letter] = d;
        used[d] = 1;

        if (assign_addend_digits(p, col, word + 1, carry, sum + d))
            return 1;

        used[d] = 0;
        digit[letter] = -1;
    }
    return 0;
}

static int solve_column(const Puzzle *p, int col, long long carry) {
    int result_len = p->word_len[p->result_word];
    int max_columns = p->max_addend_len > result_len
                    ? p->max_addend_len : result_len;

    if (col >= max_columns)
        return carry == 0;

    return assign_addend_digits(p, col, 0, carry, carry);
}

static int solve(const char *input, char *out) {
    Puzzle p;
    if (!parse_puzzle(input, &p)) return 0;

    for (int i = 0; i < MAX_LETTERS; ++i) digit[i] = -1;
    memset(used, 0, sizeof used);

    if (!solve_column(&p, 0, 0)) return 0;
    memcpy(solution, digit, sizeof solution);
    format_answer(&p, solution, out);
    return 1;
}

int main(void) {
    char input[512], output[512];
    printf("Enter puzzle\n> ");
    if (!fgets(input, sizeof input, stdin)) {
        fprintf(stderr, "Input error.\n");
        return 1;
    }
    input[strcspn(input, "\r\n")] = '\0';

    if (solve(input, output)) printf("Result: %s\n", output);
    else printf("Solution not found\n");
    return 0;
}
