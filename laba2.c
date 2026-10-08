#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LETTERS 10
#define MAX_EXPR    256

#define SEP  (-1)   // граница между словами 
#define END  (-2)   //  конец выражения 

typedef struct {
    int  expr[MAX_EXPR];
    int  expr_len;

    char unic_letters[MAX_LETTERS];
    int  letter_count;
    int  is_leading[MAX_LETTERS];

    int  total_words;
    int  word_start[MAX_EXPR];
    int  word_len[MAX_EXPR];
    int  result_word;
} Puzzle;
//  изменил парсинг слов
int parse_puzzle(const char *in, Puzzle *p) {
    p->expr_len = 0;
    p->letter_count = 0;

    char loc_buf[512];
    strncpy(loc_buf, in, sizeof(loc_buf) - 1);
    loc_buf[sizeof(loc_buf) - 1] = '\0';

    char *token = strtok(loc_buf, "=+ ");
    while (token) {
        int length = strlen(token);

        if (length == 0) {
            token = strtok(NULL, "=+ ");
            continue;
        }

        if (p->expr_len + length + 1 >= MAX_EXPR)
            return 0;

        for (int i = 0; i < length; i++) {
            char c = token[i];

            int indx = -1;
            for (int j = 0; j < p->letter_count; j++) {
                if (p->unic_letters[j] == c) {
                    indx = j;
                    break;
                }
            }

            if (indx == -1) {
                if (p->letter_count >= MAX_LETTERS)
                    return 0;
                indx = p->letter_count;
                p->unic_letters[indx] = c;
                p->is_leading[indx] = 0;
                p->letter_count++;
            }

            p->expr[p->expr_len++] = indx;
        }

        int first = p->expr[p->expr_len - length];
        p->is_leading[first] = 1;

        p->expr[p->expr_len++] = SEP;

        token = strtok(NULL, "=+ ");
    }

    if (p->expr_len == 0)
        return 0;

    p->expr[p->expr_len - 1] = END;

    p->total_words = 0;
    int k = 0;
    int i = 0;
    p->word_start[0] = 0;

    while (i < p->expr_len) {
        if (p->expr[i] == SEP || p->expr[i] == END) {
            p->word_len[k] = i - p->word_start[k];
            k++;
            p->total_words++;
            if (p->expr[i] == END) break;
            p->word_start[k] = i + 1;
        }
        i++;
    }
    p->result_word = p->total_words - 1;

    if (p->total_words < 3)        return 0;
    if (p->total_words - 1 > 7)    return 0;

    return 1;
}

int check(const Puzzle *p, const int *digit) {
    long long sum = 0;
    long long result = 0;

    for (int k = 0; k < p->total_words; k++) {
        int start = p->word_start[k];
        int len   = p->word_len[k];
        long long v = 0;
        for (int j = 0; j < len; j++)
            v = v * 10 + digit[p->expr[start + j]];

        if (k == p->result_word)
            result = v;
        else
            sum += v;
    }

    return sum == result;
}

static int used[10];
static int digit[MAX_LETTERS];
static int solution[MAX_LETTERS];

int permute(const Puzzle *p, int pos) {
    if (pos == p->letter_count) {
        if (check(p, digit)) {
            memcpy(solution, digit, sizeof(int) * p->letter_count);
            return 1;
        }
        return 0;
    }

    for (int d = 0; d <= 9; d++) {
        if (used[d]) continue;
        if (d == 0 && p->is_leading[pos]) continue;

        used[d] = 1;
        digit[pos] = d;

        if (permute(p, pos + 1)) return 1;

        used[d] = 0;
    }
    return 0;
}

void format_answer(const Puzzle *p, const int *digit, char *out) {
    char *o = out;

    for (int k = 0; k < p->total_words; k++) {
        int start = p->word_start[k];
        int len   = p->word_len[k];

        for (int j = 0; j < len; j++)
            *o++ = (char)('0' + digit[p->expr[start + j]]);

        if (k == p->result_word - 1) {
            memcpy(o, " = ", 3); o += 3;
        } else if (k != p->total_words - 1) {
            memcpy(o, " + ", 3); o += 3;
        }
    }
    *o = '\0';
}

int solve(const char *puzzle, char *out) {
    Puzzle p;
    if (!parse_puzzle(puzzle, &p)) return 0;

    for (int i = 0; i < 10; i++) used[i] = 0;

    if (!permute(&p, 0)) return 0;

    format_answer(&p, solution, out);
    return 1;
}

int main(void) {
    char input[512];
    char output[512];

    printf("enter puzzle\n> ");
    if (!fgets(input, sizeof(input), stdin)) {
        fprintf(stderr, "error input.\n");
        return 1;
    }

    size_t len = strlen(input);
    while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')){
        input[--len] = '\0';
    }
    solve(input, output);
    printf("result: %s\n", output);

    return 0;
}