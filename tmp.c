#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LETTERS 10
#define MAX_EXPR    256   /* вмещает все слова + разделители + -2 */

/* Разделители в expr[] */
#define SEP     (-1)   /* между словами (был '+' или '=') */
#define END     (-2)   /* конец выражения */

typedef struct {
    int  expr[MAX_EXPR];       // плоское выражение: индексы букв, -1, -2
    int  expr_len;             // сколько ячеек реально занято

    char letters[MAX_LETTERS]; // уникальные буквы
    int  letter_count;
    int  is_leading[MAX_LETTERS];
} Puzzle;

/* ---------- Разбор входной строки ---------- */
int parse_puzzle(const char *s, Puzzle *p) {
    p->expr_len     = 0;
    p->letter_count = 0;

    char buf[512];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tok = strtok(buf, "=+ ");
    while (tok) {
        /* слово уже без пробелов, т.к. пробел — разделитель */
        int len = strlen(tok);
        if (len == 0) { tok = strtok(NULL, "=+ "); continue; }

        if (p->expr_len + len + 1 >= MAX_EXPR) return 0;

        for (int i = 0; i < len; i++) {
            char c = tok[i];

            int idx = -1;
            for (int k = 0; k < p->letter_count; k++)
                if (p->letters[k] == c) { idx = k; break; }

            if (idx < 0) {
                if (p->letter_count >= MAX_LETTERS) return 0;
                idx = p->letter_count;
                p->letters[idx]    = c;
                p->is_leading[idx] = 0;
                p->letter_count++;
            }

            p->expr[p->expr_len++] = idx;
        }

        /* ведущая буква — первая буква слова */
        int first = p->expr[p->expr_len - len];
        p->is_leading[first] = 1;

        /* разделитель между словами */
        p->expr[p->expr_len++] = SEP;

        tok = strtok(NULL, "=+ ");
    }

    /* завершаем выражение маркером END вместо последнего SEP */
    if (p->expr_len == 0) return 0;
    p->expr[p->expr_len - 1] = END;

    /* считаем слова: сколько раз встретился SEP/END */
    int words = 0;
    for (int i = 0; i < p->expr_len; i++)
        if (p->expr[i] == SEP || p->expr[i] == END) words++;

    if (words < 3) return 0;      /* минимум 2 слагаемых + результат */
    if (words - 1 < 2) return 0;
    if (words - 1 > 7) return 0;

    return 1;
}

/* ---------- Обход выражения ---------- */

/* Возвращает значение слова, начиная с индекса start.
   Записывает в *next индекс следующего слова (после разделителя).
   Если end_word > 0 — это индекс результата, знак учитывается вызывающим. */
long long word_value_at(const Puzzle *p, int start, const int *digit, int *next) {
    long long v = 0;
    int i = start;
    while (p->expr[i] != SEP && p->expr[i] != END) {
        v = v * 10 + digit[p->expr[i]];
        i++;
    }
    *next = i + 1;    /* пропускаем разделитель */
    return v;
}

/* ---------- Проверка равенства ---------- */
int check(const Puzzle *p, const int *digit) {
    long long sum = 0;
    int i = 0;
    int result_index = -1;
    int cur = 0;

    /* считаем номер слова, чтобы знать, где результат */
    int total_words = 0;
    for (int k = 0; k < p->expr_len; k++)
        if (p->expr[k] == SEP || p->expr[k] == END) total_words++;

    /* проходим по словам: все кроме последнего — слагаемые */
    int word_no = 0;
    while (i < p->expr_len) {
        int next;
        long long v = word_value_at(p, i, digit, &next);
        if (word_no == total_words - 1)
            result_index = (int)v;   /* результат — в int хватит */
        else
            sum += v;
        i = next;
        word_no++;
    }

    return sum == (long long)result_index;
}

/* ---------- Перебор перестановок ---------- */
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

/* ---------- Формирование ответа ---------- */
void format_answer(const Puzzle *p, const int *digit, char *out) {
    out[0] = '\0';
    char tmp[16];

    /* сколько слов всего, чтобы знать, где поставить " = " */
    int total_words = 0;
    for (int k = 0; k < p->expr_len; k++)
        if (p->expr[k] == SEP || p->expr[k] == END) total_words++;

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

/* ---------- Основная функция ---------- */
int solve(const char *puzzle, char *out) {
    Puzzle p;
    if (!parse_puzzle(puzzle, &p)) return 0;

    for (int i = 0; i < 10; i++) used[i] = 0;

    if (!permute(&p, 0)) return 0;
    format_answer(&p, solution, out);
    return 1;
}

/* ---------- Ввод с клавиатуры ---------- */
int main(void) {
    char line[512];
    char out[512];

    printf("Введите числовой ребус (например, SEND + MORE = MONEY):\n> ");
    if (!fgets(line, sizeof(line), stdin)) {
        fprintf(stderr, "Ошибка чтения ввода.\n");
        return 1;
    }

    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';

    if (len == 0) {
        fprintf(stderr, "Пустой ввод.\n");
        return 1;
    }

    if (solve(line, out))
        printf("Решение: %s\n", out);
    else
        printf("Решение не найдено.\n");

    return 0;
}