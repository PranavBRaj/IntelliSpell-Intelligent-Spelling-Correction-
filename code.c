#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // for INT_MAX

#define MAX_WORD_LENGTH 50
#define NUM_SUGGESTIONS 10

typedef struct TreeNode {
    char word[MAX_WORD_LENGTH];
    struct TreeNode *children[26];
} TreeNode;

TreeNode* createNode() {
    TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node) {
        memset(node->word, 0, MAX_WORD_LENGTH);
        for (int i = 0; i < 26; i++)
            node->children[i] = NULL;
    }
    return node;
}

int min(int a, int b, int c) {
    if (a < b && a < c)
        return a;
    else if (b < c)
        return b;
    else
        return c;
}

int levenshteinDistance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);

    int dp[len1 + 1][len2 + 1];

    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            if (i == 0)
                dp[i][j] = j;
            else if (j == 0)
                dp[i][j] = i;
            else if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = 1 + min(dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]);
        }
    }

    return dp[len1][len2];
}

// Implementation of Knuth-Morris-Pratt algorithm for pattern searching
void computeLPSArray(const char *pat, int M, int *lps) {
    int len = 0;  // Length of the previous longest prefix suffix
    int i = 1;
    lps[0] = 0;  // lps[0] is always 0

    while (i < M) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int KMPSearch(const char *pat, const char *txt) {
    int M = strlen(pat);
    int N = strlen(txt);

    // Create lps[] that will hold the longest prefix suffix
    // values for pattern
    int *lps = (int *)malloc(sizeof(int) * M);
    int j = 0;  // index for pat[]

    // Preprocess the pattern (calculate lps[] array)
    computeLPSArray(pat, M, lps);

    int i = 0;  // index for txt[]
    int res = 0;
    while (i < N) {
        if (pat[j] == txt[i]) {
            j++;
            i++;
        }

        if (j == M) {
            res++;
            j = lps[j - 1];
        } else if (i < N && pat[j] != txt[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }

    free(lps);  // Free the dynamically allocated memory

    return res;
}

void insertWord(TreeNode *root, const char *word) {
    TreeNode *currentNode = root;
    int length = strlen(word);
    for (int i = 0; i < length; i++) {
        int index = word[i] - 'a';
        if (!currentNode->children[index]) {
            currentNode->children[index] = createNode();
        }
        currentNode = currentNode->children[index];
    }
    strcpy(currentNode->word, word);
}

void printWords(TreeNode *root) {
    if (root == NULL)
        return;
    if (strlen(root->word) > 0)
        printf("%s\n", root->word);
    for (int i = 0; i < 26; i++) {
        printWords(root->children[i]);
    }
}

void freeTree(TreeNode *root) {
    if (root == NULL)
        return;
    for (int i = 0; i < 26; i++) {
        freeTree(root->children[i]);
    }
    free(root);
}

void loadDictionary(TreeNode *root, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s.\n", filename);
        exit(1);
    }
    char word[MAX_WORD_LENGTH];
    while (fgets(word, MAX_WORD_LENGTH, file)) {
        // Remove newline character if present
        size_t length = strlen(word);
        if (word[length - 1] == '\n')
            word[length - 1] = '\0';
        insertWord(root, word);
    }
    fclose(file);
}

void findSuggestions(TreeNode *node, const char *word, int *minDistance, char suggestions[NUM_SUGGESTIONS][MAX_WORD_LENGTH], int *suggestionCount) {
    if (node == NULL)
        return;

    // Check if the word from this node is a suggestion
    if (strlen(node->word) > 0) {
        int distance = levenshteinDistance(word, node->word);
        if (distance < minDistance[NUM_SUGGESTIONS - 1] || (distance == minDistance[NUM_SUGGESTIONS - 1] && strlen(node->word) > strlen(suggestions[NUM_SUGGESTIONS - 1]))) {
            for (int i = NUM_SUGGESTIONS - 1; i >= 0; i--) {
                if (distance < minDistance[i] || (distance == minDistance[i] && strlen(node->word) > strlen(suggestions[i]))) {
                    if (i < NUM_SUGGESTIONS - 1) {
                        minDistance[i + 1] = minDistance[i];
                        strcpy(suggestions[i + 1], suggestions[i]);
                    }
                    minDistance[i] = distance;
                    strcpy(suggestions[i], node->word);
                    (*suggestionCount)++;
                }
            }
        }
    }

    // Recursively check suggestions from children nodes
    for (int i = 0; i < 26; i++) {
        findSuggestions(node->children[i], word, minDistance, suggestions, suggestionCount);
    }

    // Check for suggestions using Knuth-Morris-Pratt algorithm
    int lenWord = strlen(word);
    for (int i = 0; i < lenWord; i++) {
        char newWord[MAX_WORD_LENGTH];
        strcpy(newWord, word);
        for (char c = 'a'; c <= 'z'; c++) {
            newWord[i] = c;
            int occurrences = KMPSearch(newWord, node->word);
            if (occurrences > 0) {
                int distance = levenshteinDistance(newWord, node->word);
                if (distance < minDistance[NUM_SUGGESTIONS - 1] || (distance == minDistance[NUM_SUGGESTIONS - 1] && strlen(node->word) > strlen(suggestions[NUM_SUGGESTIONS - 1]))) {
                    for (int i = NUM_SUGGESTIONS - 1; i >= 0; i--) {
                        if (distance < minDistance[i] || (distance == minDistance[i] && strlen(node->word) > strlen(suggestions[i]))) {
                            if (i < NUM_SUGGESTIONS - 1) {
                                minDistance[i + 1] = minDistance[i];
                                strcpy(suggestions[i + 1], suggestions[i]);
                            }
                            minDistance[i] = distance;
                            strcpy(suggestions[i], node->word);
                            (*suggestionCount)++;
                        }
                    }
                }
            }
        }
    }
}

void suggestWords(TreeNode *root, const char *word) {
    int minDistance[NUM_SUGGESTIONS] = {INT_MAX, INT_MAX, INT_MAX};
    char suggestions[NUM_SUGGESTIONS][MAX_WORD_LENGTH];
    int suggestionCount = 0;

    for (int i = 0; i < NUM_SUGGESTIONS; i++) {
        suggestions[i][0] = '\0'; // Initialize suggestion strings with null terminator
    }

    findSuggestions(root, word, minDistance, suggestions, &suggestionCount);

    if (suggestionCount == 0) {
        printf("No suggestions found.\n");
    } else {
        printf("Top suggestions:\n");
        for (int i = 0; i < NUM_SUGGESTIONS; i++) {
            if (strlen(suggestions[i]) > 0) // Check if suggestion string is not empty
                printf("%s\n", suggestions[i]);
        }
    }
}

int main() {
    TreeNode *root = createNode();
    loadDictionary(root, "dictionary.txt");

    char inputWord[MAX_WORD_LENGTH];
    while (1) {
        printf("Enter a word (or type 'exit' to quit): ");
        scanf("%s", inputWord);
        if (strcmp(inputWord, "exit") == 0) {
            break; // Exit loop if the user types 'exit'
        }
        suggestWords(root, inputWord);
    }

    freeTree(root);
    return 0;
}
