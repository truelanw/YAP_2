#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LETTERS 10
#define MAX_EXPR    256

#define SEP  (-1)   /* граница между словами */
#define END  (-2)   /* конец выражения */

typedef struct {
    int  expr[MAX_EXPR];ЫЫ
    int  expr_len;

    char unic_letters[MAX_LETTERS]; /* уникальные буквы */
    int  letter_count;
    int  is_leading[MAX_LETTERS];
} Puzzle;

int parse_puzzle(const char *in, Puzzle *p) {
    p->expr_len     = 0;
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
                p->unic_letters[indx]    = c;
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

    /* считаем слова */
    int words = 0;
    for (int i = 0; i < p->expr_len; i++)
        if (p->expr[i] == SEP || p->expr[i] == END)
            words++;

    if (words < 3)        return 0;
    if (words - 1 > 7)    return 0;

    return 1;
}

long long word_value_at(const Puzzle *p, int start, const int *digit, int *next) {
    long long v = 0;
    int i = start;
    while (p->expr[i] != SEP && p->expr[i] != END) {
        v = v * 10 + digit[p->expr[i]];
        i++;
    }
    *next = i + 1;
    return v;
}


int check(const Puzzle *p, const int *digit) {
    int total_words = 0;
    for (int k = 0; k < p->expr_len; k++)
        if (p->expr[k] == SEP || p->expr[k] == END)
            total_words++;

    long long sum = 0;
    long long result = 0;
    int word_no = 0;
    int i = 0;

    while (i < p->expr_len) {
        int next;
        long long v = word_value_at(p, i, digit, &next);

        if (word_no == total_words - 1)
            result = v;
        else
            sum += v;

        i = next;
        word_no++;
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
    out[0] = '\0';
    char tmp[16];

    int total_words = 0;
    for (int k = 0; k < p->expr_len; k++)
        if (p->expr[k] == SEP || p->expr[k] == END)
            total_words++;

    int word_no = 0;
    for (int i = 0; i < p->expr_len; i++) {
        int e = p->expr[i];

        if (e == SEP || e == END) {
            word_no++;
            if (e == END) break;
            if (word_no == total_words - 1)
                strcat(out, " = ");
            else
                strcat(out, " + ");
        } else {
            sprintf(tmp, "%d", digit[e]);
            strcat(out, tmp);
        }
    }
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