#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define DATA_FILE "teachers.dat"
#define MAX_ID 20
#define MAX_NAME 40
#define MAX_DEPT 40
#define MAX_TITLE 40
#define INPUT_BUF 256

typedef struct {
    char id[MAX_ID];
    char name[MAX_NAME];
    char department[MAX_DEPT];
    char title[MAX_TITLE];
    double baseSalary;
    double allowance;
    double bonus;
    double deduction;
} Teacher;

typedef struct ResultNode {
    Teacher data;
    struct ResultNode *next;
} ResultNode;

static void setupConsole(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

static void trimNewline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

static void readLine(const char *prompt, char *buf, size_t size) {
    for (;;) {
        printf("%s", prompt);
        if (fgets(buf, (int)size, stdin) == NULL) {
            clearerr(stdin);
            buf[0] = '\0';
            continue;
        }
        trimNewline(buf);
        if (strlen(buf) > 0) {
            return;
        }
        printf("输入不能为空，请重新输入。\n");
    }
}

static int readInt(const char *prompt, int min, int max) {
    char buf[INPUT_BUF];
    char *end = NULL;
    long value;
    for (;;) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        value = strtol(buf, &end, 10);
        while (end != NULL && isspace((unsigned char)*end)) {
            end++;
        }
        if (end != buf && end != NULL && *end == '\0' && value >= min && value <= max) {
            return (int)value;
        }
        printf("请输入 %d 到 %d 之间的整数。\n", min, max);
    }
}

static double readDouble(const char *prompt, double min) {
    char buf[INPUT_BUF];
    char *end = NULL;
    double value;
    for (;;) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        value = strtod(buf, &end);
        while (end != NULL && isspace((unsigned char)*end)) {
            end++;
        }
        if (end != buf && end != NULL && *end == '\0' && value >= min) {
            return value;
        }
        printf("请输入不小于 %.2f 的数字。\n", min);
    }
}

static double payableSalary(const Teacher *t) {
    return t->baseSalary + t->allowance + t->bonus - t->deduction;
}

static int fileExists(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

static int isDuplicateId(const char *id) {
    FILE *fp = fopen(DATA_FILE, "rb");
    Teacher t;
    if (fp == NULL) {
        return 0;
    }
    while (fread(&t, sizeof(Teacher), 1, fp) == 1) {
        if (strcmp(t.id, id) == 0) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static void printHeader(void) {
    printf("%-12s %-14s %-14s %-14s %10s %10s %10s %10s %10s\n",
           "编号", "姓名", "部门", "职称", "基本工资", "津贴", "奖金", "扣款", "应发工资");
    printf("----------------------------------------------------------------------------------------------------------------\n");
}

static void printTeacher(const Teacher *t) {
    printf("%-12s %-14s %-14s %-14s %10.2f %10.2f %10.2f %10.2f %10.2f\n",
           t->id, t->name, t->department, t->title, t->baseSalary,
           t->allowance, t->bonus, t->deduction, payableSalary(t));
}

static ResultNode *appendResult(ResultNode *head, const Teacher *t) {
    ResultNode *node = (ResultNode *)malloc(sizeof(ResultNode));
    ResultNode *cur;
    if (node == NULL) {
        printf("内存分配失败，无法保存查询结果。\n");
        return head;
    }
    node->data = *t;
    node->next = NULL;
    if (head == NULL) {
        return node;
    }
    cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = node;
    return head;
}

static void freeResults(ResultNode *head) {
    while (head != NULL) {
        ResultNode *next = head->next;
        free(head);
        head = next;
    }
}

static int printResults(ResultNode *head) {
    int count = 0;
    if (head == NULL) {
        printf("未找到匹配的教师工资信息。\n");
        return 0;
    }
    printHeader();
    while (head != NULL) {
        printTeacher(&head->data);
        head = head->next;
        count++;
    }
    printf("共查询到 %d 条记录，以上结果已通过链表保存并输出。\n", count);
    return count;
}

static void addTeacher(void) {
    Teacher t;
    FILE *fp;
    readLine("教师编号：", t.id, sizeof(t.id));
    if (isDuplicateId(t.id)) {
        printf("编号 %s 已存在，录入失败。\n", t.id);
        return;
    }
    readLine("姓名：", t.name, sizeof(t.name));
    readLine("所在部门：", t.department, sizeof(t.department));
    readLine("职称：", t.title, sizeof(t.title));
    t.baseSalary = readDouble("基本工资：", 0.0);
    t.allowance = readDouble("津贴：", 0.0);
    t.bonus = readDouble("奖金：", 0.0);
    t.deduction = readDouble("扣款：", 0.0);

    fp = fopen(DATA_FILE, "ab");
    if (fp == NULL) {
        printf("无法打开数据文件 %s。\n", DATA_FILE);
        return;
    }
    if (fwrite(&t, sizeof(Teacher), 1, fp) != 1) {
        printf("写入数据失败。\n");
    } else {
        printf("教师工资信息录入成功。\n");
    }
    fclose(fp);
}

static ResultNode *queryById(const char *id) {
    FILE *fp = fopen(DATA_FILE, "rb");
    Teacher t;
    ResultNode *head = NULL;
    if (fp == NULL) {
        return NULL;
    }
    while (fread(&t, sizeof(Teacher), 1, fp) == 1) {
        if (strcmp(t.id, id) == 0) {
            head = appendResult(head, &t);
        }
    }
    fclose(fp);
    return head;
}

static ResultNode *queryByDepartment(const char *department) {
    FILE *fp = fopen(DATA_FILE, "rb");
    Teacher t;
    ResultNode *head = NULL;
    if (fp == NULL) {
        return NULL;
    }
    while (fread(&t, sizeof(Teacher), 1, fp) == 1) {
        if (strcmp(t.department, department) == 0) {
            head = appendResult(head, &t);
        }
    }
    fclose(fp);
    return head;
}

static void queryMenu(void) {
    int choice = readInt("1.按教师编号查询  2.按所在部门查询：", 1, 2);
    char key[INPUT_BUF];
    ResultNode *results = NULL;
    if (!fileExists()) {
        printf("暂无数据，请先录入教师工资信息。\n");
        return;
    }
    if (choice == 1) {
        readLine("请输入教师编号：", key, sizeof(key));
        results = queryById(key);
    } else {
        readLine("请输入所在部门：", key, sizeof(key));
        results = queryByDepartment(key);
    }
    printResults(results);
    freeResults(results);
}

static void listAll(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    Teacher t;
    int count = 0;
    if (fp == NULL) {
        printf("暂无数据，请先录入教师工资信息。\n");
        return;
    }
    printHeader();
    while (fread(&t, sizeof(Teacher), 1, fp) == 1) {
        printTeacher(&t);
        count++;
    }
    fclose(fp);
    printf("共 %d 条记录。\n", count);
}

static void statistics(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    Teacher t;
    int count = 0;
    double total = 0.0;
    double maxSalary = 0.0;
    double minSalary = 0.0;
    Teacher maxTeacher;
    Teacher minTeacher;
    char department[INPUT_BUF];
    int scope;

    if (fp == NULL) {
        printf("暂无数据，请先录入教师工资信息。\n");
        return;
    }

    scope = readInt("统计范围：1.全部教师  2.指定部门：", 1, 2);
    if (scope == 2) {
        readLine("请输入部门：", department, sizeof(department));
    }

    while (fread(&t, sizeof(Teacher), 1, fp) == 1) {
        double salary;
        if (scope == 2 && strcmp(t.department, department) != 0) {
            continue;
        }
        salary = payableSalary(&t);
        if (count == 0 || salary > maxSalary) {
            maxSalary = salary;
            maxTeacher = t;
        }
        if (count == 0 || salary < minSalary) {
            minSalary = salary;
            minTeacher = t;
        }
        total += salary;
        count++;
    }
    fclose(fp);

    if (count == 0) {
        printf("统计范围内没有记录。\n");
        return;
    }
    printf("统计人数：%d\n", count);
    printf("应发工资总额：%.2f\n", total);
    printf("应发工资平均值：%.2f\n", total / count);
    printf("最高应发工资：%.2f（%s %s）\n", maxSalary, maxTeacher.id, maxTeacher.name);
    printf("最低应发工资：%.2f（%s %s）\n", minSalary, minTeacher.id, minTeacher.name);
}

static void modifyTeacher(void) {
    char id[MAX_ID];
    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *tmp;
    Teacher t;
    int found = 0;

    if (fp == NULL) {
        printf("暂无数据，请先录入教师工资信息。\n");
        return;
    }
    tmp = fopen("teachers.tmp", "wb");
    if (tmp == NULL) {
        fclose(fp);
        printf("无法创建临时文件。\n");
        return;
    }

    readLine("请输入要修改的教师编号：", id, sizeof(id));
    while (fread(&t, sizeof(Teacher), 1, fp) == 1) {
        if (strcmp(t.id, id) == 0) {
            found = 1;
            printf("当前信息：\n");
            printHeader();
            printTeacher(&t);
            printf("请输入修改后的信息（编号保持不变）。\n");
            readLine("姓名：", t.name, sizeof(t.name));
            readLine("所在部门：", t.department, sizeof(t.department));
            readLine("职称：", t.title, sizeof(t.title));
            t.baseSalary = readDouble("基本工资：", 0.0);
            t.allowance = readDouble("津贴：", 0.0);
            t.bonus = readDouble("奖金：", 0.0);
            t.deduction = readDouble("扣款：", 0.0);
        }
        if (fwrite(&t, sizeof(Teacher), 1, tmp) != 1) {
            printf("写入临时文件失败。\n");
            fclose(fp);
            fclose(tmp);
            remove("teachers.tmp");
            return;
        }
    }
    fclose(fp);
    fclose(tmp);

    if (!found) {
        remove("teachers.tmp");
        printf("未找到编号为 %s 的教师。\n", id);
        return;
    }
    if (remove(DATA_FILE) != 0 || rename("teachers.tmp", DATA_FILE) != 0) {
        printf("替换数据文件失败，请检查文件权限。\n");
        return;
    }
    printf("教师工资信息修改成功。\n");
}

static void showRequirementAnalysis(void) {
    printf("\n需求分析概要：\n");
    printf("1. 用户：教师、人事/教务管理员、财务处。\n");
    printf("2. 数据：教师编号、姓名、部门、职称、基本工资、津贴、奖金、扣款、应发工资。\n");
    printf("3. 功能：录入并保存、按编号/部门查询、链表保存查询结果、统计、职称晋升后修改工资。\n");
    printf("4. 质量要求：编号唯一、输入校验、文件持久化、Windows UTF-8 控制台显示。\n\n");
}

static void menu(void) {
    printf("\n========== 教师工资管理系统 ==========" "\n");
    printf("1. 录入教师工资信息\n");
    printf("2. 查询教师工资信息\n");
    printf("3. 统计教师工资\n");
    printf("4. 修改教师工资信息\n");
    printf("5. 浏览全部记录\n");
    printf("6. 查看需求分析概要\n");
    printf("0. 退出系统\n");
}

int main(void) {
    int choice;
    setupConsole();
    printf("教师工资管理系统（数据文件：%s）\n", DATA_FILE);
    for (;;) {
        menu();
        choice = readInt("请选择功能：", 0, 6);
        switch (choice) {
            case 1:
                addTeacher();
                break;
            case 2:
                queryMenu();
                break;
            case 3:
                statistics();
                break;
            case 4:
                modifyTeacher();
                break;
            case 5:
                listAll();
                break;
            case 6:
                showRequirementAnalysis();
                break;
            case 0:
                printf("感谢使用，再见！\n");
                return 0;
            default:
                break;
        }
    }
}
