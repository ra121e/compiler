#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 列挙型の定数の設定
typedef enum{
	TK_RESERVED,	// 記号
	TK_NUM,			// 整数トークン
	TK_EOF,			// 終端トークン
}	TokenKind;

// 自己参照構造体を使うためstruct Tokenを単にTokenとするエイリアス
typedef struct Token Token;

// トークン用の構造体。Token型のメンバポインタで連結する
struct Token{
	TokenKind kind;	// トークンの列挙タイプ
	Token *next;	// トークンの連結用ポインタ
	int val;		// 数字の場合の値
	char *str;		// トークンの文字
};

// グローバル変数として注目中のトークンの位置を示すポインタの宣言
Token *token;

void error(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 以下の３つは注目しているトークンを次に移すことがメイン
// トークンを読み進める時のエラーチェックのようなもの　＋の場合
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

// トークンを読み進める時のエラーチェックのようなもの　－の場合
void expect(char op){
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error("'%c'ではありません", op);
	token = token->next;
}

// アセンブリを出力ためにトークンから数値を取得する。
int expect_number(){
	if (token->kind != TK_NUM)
		error("数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

// トークンの終端確認
bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンの作成と格納。現在のトークンに新トークンの位置を代入
Token *new_token(TokenKind kind, Token *cur, char *str){
	Token *tok = calloc(1, sizeof(Token));	// メモリ確保
	tok->kind = kind;	// 新トークンに種類データの格納
	tok->str = str;		// 新トークンに文字データを格納
	cur->next = tok;	// 前トークンの自己参照に新トークンの位置
	return tok;			// 新トークンの位置を返す
}

// トークナイズ。引数の文字列pを受け取り新トークンの作成と値の格納
Token *tokenize(char *p){
	Token head;			// 先頭のトークンの作成
	head.next = NULL;	// 先頭トークンの自己参照をNULLで初期化
	Token *cur = &head;	// トークン列の位置を先頭トークンに設定

	while (*p){
		// 空白文字をスキップ
		if (isspace(*p)){
			p++;
			continue;
		}

		if (*p == '+' || *p == '-'){
			cur = new_token(TK_RESERVED, cur, p);
			p++;
			continue;
		}

		if (isdigit(*p)){
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error("トークナイズできません");
	}

	new_token(TK_EOF, cur, p);	// 終端トークンの設定
	return head.next;			// トークン列の先頭の位置を返す
}
int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数がただしくありません");
		return 1;
	}

	// トークナイズに引数を受け渡して、トークン列の先頭を受け取る。
	token = tokenize(argv[1]);

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// 一つ目のトークンが数であるかチェックして数値を取得、出力
	printf("  mov rax, %d\n", expect_number());

	// 前回は引数の文字列を直接評価していたが今回はトークン
	// 前回はp++エンジン。今回はconsumeとexpectの外部エンジン
	// 終了条件は終端文字から終端トークン
	while (!at_eof())
	{
		if (consume('+'))
		{
			printf("  add rax, %d\n", expect_number());
			continue;
		}
		expect('-');
		printf("  sub rax, %d\n", expect_number());
	}

	printf("  ret\n");
	return 0;
}