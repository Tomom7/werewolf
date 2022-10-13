// 「get_int」、「get_string」など
#include <cs50.h>
// 「printf」、「strcmp」
#include <stdio.h>
// 「sizeof」を使うため使用
#include <stdlib.h>
// 「strcmp」
#include <string.h>
// 「bool」(構造体の中に「bool」があってこのコードがなくても「make」でエラーにならなかった・・・)
#include <stdbool.h>


// 人数制限の定数
const int PERSON_LIMIT = 12;
// 格納するデータの文字制限の定数
// 現在、10文字以上書いてもセグメントエラーにならない・・・
const int CHARACTER_LIMIT = 10;

typedef struct
{
    char *name;
    // 現在、「villager(村人)」、「seer(占い師)」、「werewolf(人狼)」、「(white)(白陣営)」、「(unclear)」の４通り
    char *position;
    bool executed;
    bool bitten;
    // 現在、「seer(占い師)」、「(unclear)」の2通り。今後増える。
    char *say_position;
    // ある人に対する占い結果は「white(白)」、「brack(黒)」、「(unclear)」の３通りあり、bool型では不満足
    char *divination[PERSON_LIMIT];
}
person;

// 残りの職業の内訳。主に占い師を宣言した人狼の矛盾(その人を信じると人狼の部屋の設定ではありえない結果となる)を見つけるのに使う。
// 配列でも数字を格納することができるが、こっちの方がnum[0]とかよりも分かりやすそう・・・
typedef struct
{
    int all;
    int seer;
    int werewolf;
    int villager;
    // 占い師を宣言しているが職業が決まっていない人の数
    int say_seer;
}
remain, remain_tmp;

// 部屋情報の入力
int input_number_people();
int input_number_seer(int person_count);
int input_number_werewolf(int person_count);
int input_number_villager(int person_count);

// 個人情報の入力
void input_name(int person_count, person *persons);
void input_executed(int person_count, person *persons);
void input_bitten(int person_count, person *persons);
void input_say_position(int person_count, person *persons);
void input_position(int person_count, int werewolf_count, person *persons, remain *remains);
int input_number_say_seer(int person_count, person *persons);
void input_divination(int person_count, person *persons);

// 入力内容の出力
void print_input_room(int person_count, int seer_count, int werewolf_count, int villager_count);
void print_input_person(int person_count, person *persons);

// 構造体「remain」の減算
void minus_remain(char *x, char *y, remain *remains);
// 真と確定した占い師の占い結果を参照
void true_seer(int person_count, int person_number, person *persons, remain *remains);
void check_remain_seer(int person_count, person *persons, remain *remains);
void infer_bitten(int person_count, person *persons, remain *remains);
void check_remain_no_seer(int person_count, person *persons, remain *remains);

void print_remain(remain *remains);
void print_output(int person_count, person *persons);



int main()
{
    // 部屋の人数を入力させる
    int person_count = input_number_people();
    // 構造体「person」のポインタ「persons」のメモリを「malloc」で確保する
    person *persons = (person*)malloc(person_count * (sizeof(*persons)));
    // ついでに残りの職業の数を格納する構造体と、その構造体を一時的に退避するための構造体のアドレスを設定する。
    remain *remains = (remain*)malloc(sizeof(*remains));
    remain_tmp *remains_tmp = (remain_tmp*)malloc(sizeof(*remains_tmp));

    // 占い師、人狼の人数をそれぞれ入力させ、村人の人数を確定させる
    int seer_count = 0;
    int werewolf_count = 0;
    int villager_count = 0;
    bool room_clear = false;
    while (room_clear == false)
    {
        seer_count = input_number_seer(person_count);
        werewolf_count = input_number_werewolf(person_count);
        villager_count = input_number_villager(person_count);
        if (person_count == seer_count + werewolf_count + villager_count)
        {
            room_clear = true;
        }
        else
        {
            printf("(エラー：部屋の役職の内訳が不適当です)\n");
        }
    }

    // 職業ごとの数の情報を構造体のアドレス「remains」を参照し格納
    remains->all = person_count;
    remains->seer = seer_count;
    remains->werewolf = werewolf_count;
    remains->villager = villager_count;

    // 名前、処刑された人、噛まれた人、宣言役職、自分の役職をそれぞれ入力させる
    input_name(person_count, persons);
    input_executed(person_count, persons);
    input_bitten(person_count, persons);
    input_say_position(person_count, persons);
    remains->say_seer = input_number_say_seer(person_count, persons);
    input_position(person_count, werewolf_count, persons, remains);

    // 占い結果を入力させる
    input_divination(person_count, persons);

    //入力を表示する
    printf("\n{入力(部屋情報)}\n");
    print_input_room(person_count, seer_count, werewolf_count, villager_count);
    printf("\n{入力(個人情報)}\n");
    print_input_person(person_count, persons);

    // 自分の職業によって分岐
    // 現在、占い師のみ
    if (strcmp(persons->position, "seer") == 0)
    {
        // 占いを見る占い師の限定
        int person_number = 0;
        true_seer(person_count, person_number, persons, remains);
        infer_bitten(person_count, persons, remains);
        check_remain_seer(person_count, persons, remains);
        // 占い師宣言者で職業不明が残っているかの分岐
        if (remains->say_seer != 0)
        {
            // ここに来るということは、真偽が分からない占い師が複数人おり、さらに真占い師１人以上かつ人狼１人以上
            // 無限ループを避けるための「変更カウンタ」の設置
            int change_count = -1;
            while(change_count != 0)
            {
                change_count = 0;
                {
                    // 真偽不明の占い師の矛盾あるかどうかチェック
                    // チェックする前に、現在の*remains構造体のデータを退避する必要がある
                    *remains_tmp = *remains;
                    // check_contradiction();
                    // 退避したデータの取り込み
                    *remains = *remains_tmp;
                }
            }
        }
        else
        {
            // 現在、この２つは排反とみていい
            infer_bitten(person_count, persons, remains);
            check_remain_no_seer(person_count, persons, remains);
        }
    }

    //結果を表示する
    printf("\n{出力}\n");
    // print_remain(remains);
    print_output(person_count, persons);

    //メモリの開放
    free(persons);
    free(remains);
    free(remains_tmp);
}


int input_number_people()
{
    int person_count = 0;
    while (person_count < 3 || person_count > PERSON_LIMIT)
    {
        person_count = get_int("部屋の人数は？\n");
        if (person_count < 3 || person_count > PERSON_LIMIT)
        {
            printf("(エラー：部屋の人数は３人以上１２人以下です)\n");
        }
    }
    return(person_count);
}


int input_number_seer(int person_count)
{
    int seer_count = -1;
    while (seer_count < 0 || seer_count > person_count - 1)
    {
        seer_count = get_int("占い師の人数は？\n");
        if (seer_count < 0 || seer_count > person_count - 1)
        {
            printf("(エラー：占い師の数が不適当です)\n");
        }
    }
    return(seer_count);
}


int input_number_werewolf(int person_count)
{
    int werewolf_count = 0;
    // 人狼の数は１人以上かつ部屋の人数の半分未満でないとゲームにならない(今のところ)
    while (werewolf_count < 1 || werewolf_count * 2 > person_count)
    {
        werewolf_count = get_int("人狼の人数は？\n");
        if (werewolf_count < 1 || werewolf_count * 2 > person_count)
        {
            printf("%i\n", person_count / 2);
            printf("(エラー：人狼の数が不適当です)\n");
        }
    }
    return(werewolf_count);
}


int input_number_villager(int person_count)
{
    int villager_count = -1;
    // 人狼の数は１人以上かつ部屋の人数の半分未満でないとゲームにならない(今のところ)
    while (villager_count < 0 || villager_count > person_count - 1)
    {
        villager_count = get_int("村人の人数は？\n");
        if (villager_count < 0 || villager_count > person_count - 1)
        {
            printf("(エラー：村人の数が不適当です)\n");
        }
    }
    return(villager_count);
}

void input_name(int person_count, person *persons)
{
    for (int i = 0; i < person_count; i++)
    {
        (persons+i)->name = (char*)malloc(sizeof(CHARACTER_LIMIT + 1));
        if (i == 0)
        {
            // ドット演算子ではなくアロー演算子の方が今後の発展性があるはず
            persons->name = get_string("あなた(もしくは1番目の人)の名前は？\n");
        }
        else
        {
            (persons+i)->name = get_string("%i番目の人の名前は？\n", i + 1);
        }
        // 名前が無記入であることに対してエラーを出す
        // 今後、スペースだけなどに対してもエラーを出す
        if (strcmp((persons+i)->name , "") == 0)
        {
            printf("(エラー：名前が無記入です)\n");
            i--;
            continue;
        }
        // 名前の重複に対してエラーを出す
        for (int j = 0; j < i; j++)
        {
            if (strcmp((persons+j)->name, (persons+i)->name) == 0)
            {
                printf("(エラー：%sの名前は既に存在します)\n", (persons+j)->name);
                i--;
                continue;
            }
        }
    }
}


void input_executed(int person_count, person *persons)
{
    // すべての人の処刑情報を(処刑していない)と初期設定する
    for (int i = 0; i < person_count; i++)
    {
        (persons+i)->executed = false;
    }
    //以後、処刑された人を構造体「person」に配列させる
    int count_sum = -1;
    while(count_sum < 0 || count_sum > person_count)
    {
        count_sum = get_int("何人処刑された？\n");
    }
    if (count_sum == 0)
    {
        return;
    }
    int count = 0;
    while (count < count_sum)
    {
        char *executed_name = get_string("処刑された人の名前は？(%i人中%i人目)\n", count_sum, count + 1);
        for (int i = 0; i < person_count; i++)
        {
            if (strcmp(executed_name, (persons+i)->name) == 0)
            {
                if ((persons+i)->executed == true)
                {
                    // このエラーはWebアプリでは必要がなくなるかも
                    printf("エラー：すでにその人の名前は入力してあります");
                    continue;
                }
                (persons+i)->executed = true;
                count++;
                printf("(名前の存在を確認)\n");
            }
        }
    }
}


void input_bitten(int person_count, person *persons)
{
    // すべての人の処刑情報を(処刑していない)と初期設定する
    for (int i = 0; i < person_count; i++)
    {
        (persons+i)->bitten = false;
    }

    //以後、噛まれた人を構造体「person」に配列させる
    int count_sum = -1;
    while(count_sum < 0 || count_sum > person_count)
    {
        count_sum = get_int("何人人狼に嚙まれた？\n");
    }
    if (count_sum == 0)
    {
        return;
    }
    int count = 0;
    while (count < count_sum)
    {
        char *hang_name = get_string("噛まれた人の名前は？(%i人中%i人目)\n", count_sum, count + 1);
        for (int i = 0; i < person_count; i++)
        {
            if (strcmp(hang_name, (persons+i)->name) == 0)
            {
                if ((persons+i)->bitten == true)
                {
                    // このエラーはWebアプリでは必要がなくなるかも
                    printf("エラー：すでにその人の名前は入力してあります");
                    continue;
                }
                (persons+i)->bitten = true;
                count++;
                printf("(名前の存在を確認)\n");
            }
        }
    }
}


void input_say_position(int person_count, person *persons)
{
    for (int i = 0; i < person_count; i++)
    {
        (persons+i)->say_position = (char*)malloc(sizeof(CHARACTER_LIMIT + 1));
        (persons+i)->say_position = "(unclear)";
        (persons+i)->say_position = get_string("%sの宣言役職は？\n", (persons+i)->name);
        // 現在、宣言役職に「villager」、「werewolf」は対応していない。「seer」のみ。
        if (strcmp((persons+i)->say_position , "seer") != 0 )
        {
            (persons+i)->say_position = "(unclear)";
        }
    }
}


void input_position(int person_count, int werewolf_count, person *persons, remain *remains)
{
    for (int i = 0; i < person_count; i++)
    {
        (persons+i)->position = (char*)malloc(sizeof(CHARACTER_LIMIT + 1));
        // 一旦初期設定
        (persons+i)->position = "(unclear)";
    }
    // 自分の役職を入力させる
    persons->position = get_string("%sの役職は？\n", persons->name);

    // 構造体「remain」の減算
    minus_remain(persons->position, persons->say_position, remains);
 }


int input_number_say_seer(int person_count, person *persons)
{
    int count = 0;
    for (int i = 0; i < person_count; i++)
    {
        if (strcmp((persons+i)->say_position, "seer") == 0)
        {
            count++;
        }
    }
    return(count);
}


void input_divination(int person_count, person *persons)
{
    for (int i = 0; i < person_count; i++)
    {
        for (int j = 0; j < person_count; j++)
        {
            // メモリの確保&初期設定
            (persons+i)->divination[j] = (char*)malloc(sizeof(CHARACTER_LIMIT + 1));
            (persons+i)->divination[j] = "(unclear)";
        }
    }
    for (int i = 0; i < person_count; i++)
    {
        // 「strcmp」で文字列比較
        // 「strcmp」の比較対象に「NULL」はいれない！
        if (strcmp((persons+i)->say_position, "seer") == 0)
        {
            for (int j = 0; j < person_count; j++)
            {
                // 占う人と占われる人は同一人物になりえない
                int seer_number = i;
                int divined_number = j;
                if (seer_number == divined_number)
                {
                    continue;
                }
                (persons+i)->divination[j]
                    = get_string("%sは%sを占ってなんと出たと言っている？\n", (persons+i)->name, (persons+j)->name);
                if (strcmp((persons+i)->divination[j] , "white") != 0 && strcmp((persons+i)->divination[j] , "brack") != 0)
                {
                    (persons+i)->divination[j] = "(unclear)";
                }
            }
        }
    }
}


void print_input_room(int person_count, int seer_count, int werewolf_count, int villager_count)
{
    printf("部屋の総人数は%i人\n", person_count);
    printf("占い師の人数は%i人\n", seer_count);
    printf("人狼の人数は%i人\n", werewolf_count);
    printf("村人の人数は%i人\n", villager_count);
}

void print_input_person(int person_count, person *persons)
{
    for (int i = 0; i < person_count; i++)
    {
        printf("%d番目の人の名前は%s\n",
            i + 1, (persons+i)->name);
    }
    for (int i = 0; i < person_count; i++)
    {
        if ((persons+i)->executed == true)
        {
            printf("%sは処刑された\n",(persons+i)->name);
        }
    }
    for (int i = 0; i < person_count; i++)
    {
        if ((persons+i)->bitten == true)
        {
            printf("%sは人狼に噛まれた\n",(persons+i)->name);
        }
    }
    for (int i = 0; i < person_count; i++)
    {
        if (strcmp((persons+i)->say_position , "(unclear)") != 0)
        {
            printf("%sの宣言役職は%s\n",
                (persons+i)->name, (persons+i)->say_position);
        }
    }
    for (int i = 0; i < person_count; i++)
    {
        for (int j = 0; j < person_count; j++)
        {
            if(strcmp((persons+i)->divination[j] , "(unclear)") != 0)
            {
                printf("%sは%sを占って%sとでた\n",
                (persons+i)->name, (persons+j)->name, (persons+i)->divination[j]);
            }
        }
    }
    printf("%sの役職は%s\n",persons->name, persons->position);
}


void minus_remain(char *x, char *y, remain *remains)
{
    // 入力は正しいとしている
    // 残り職業の減算
    remains->all--;
    if (strcmp(x, "seer") == 0)
    {
        remains->seer--;
    }
    else if (strcmp(x, "werewolf") == 0)
    {
        remains->werewolf--;
    }
    else if (strcmp(x, "villager") == 0)
    {
        remains->villager--;
    }
    // 残り占いCO者の減算
    if (strcmp(y,"seer") == 0)
    {
        remains->say_seer--;
    }
}


//真占い師の占いにより得られた情報を構造体に格納
void true_seer(int person_count, int person_number, person *persons, remain *remains)
{
    for (int i = 0; i < person_count; i++)
    {
        // 「person_number」は占い師の番号、「i」は占われる人の番号
        // 「i」の人を「white」と占った場合、占い師COをしたかどうかの情報さえあれば、占い師or人狼が確定！！
        if (strcmp((persons+person_number)->divination[i] , "white") == 0)
        {
            // 真占いが占い師CO者を占った場合
            if (strcmp((persons+i)->say_position , "seer") == 0)
            {
                // 現在、占い結果の入力は完全信用・・・
                // 新たな真占い師が誕生した場合
                if (strcmp((persons+i)->position , "seer") != 0)
                {
                    (persons+i)->position = "seer";
                    minus_remain((persons+i)->position, (persons+i)->say_position, remains);
                    // 占い師の対象を変え(0→(何か))、再帰！
                    // なお、元の対象は「tmp」に記憶しておく
                    int tmp = person_number;
                    person_number = i;
                    true_seer(person_count, person_number, persons, remains);
                    person_number = tmp;
                }
            }
            else
            {
                if (strcmp((persons+i)->position , "villager") != 0)
                {
                    (persons+i)->position = "villager";
                    minus_remain((persons+i)->position,(persons+i)->say_position, remains);
                }
            }
        }
        // 「i」の人を「brack」と占った場合、人狼確定！！
        if (strcmp((persons+person_number)->divination[i] , "brack") == 0)
        {
            if (strcmp((persons+i)->position , "werewolf") != 0)
            {
                (persons+i)->position = "werewolf";
                minus_remain((persons+i)->position,(persons+i)->say_position, remains);
            }
        }
    }
}


void check_remain_seer(int person_count, person *persons, remain *remains)
{
    // 既に真占い師が売り切れ→残りは人狼
    if (remains->seer == 0)
    {
        for(int i = 0; i < person_count; i++)
        {
            if (strcmp((persons+i)->say_position, "seer") == 0 && strcmp((persons+i)->position, "(unclear)") == 0)
            {
                (persons+i)->position = "werewolf";
                minus_remain((persons+i)->position,(persons+i)->say_position, remains);
            }
        }
    }
    // 残っている占い師と残っている占い師COが一緒の時、全員真占い師
    // もちろん、真占い師は全員COするものとしてる！
    else if (remains->seer == remains->say_seer)
    {
        for(int i = 0; i < person_count; i++)
        {
            if (strcmp((persons+i)->say_position, "seer") == 0 && strcmp((persons+i)->position, "(unclear)") == 0)
            {
                (persons+i)->position = "seer";
                minus_remain((persons+i)->position,(persons+i)->say_position, remains);
            }
        }
    }
}


// 噛まれた人は人狼ではないことから情報を得る
void infer_bitten(int person_count, person *persons, remain *remains)
{
    for (int i = 0; i < person_count; i++)
    {
        if ((persons+i)->bitten == true)
        {
            // 現在、噛まれた人の職業が確定する
            // 噛まれた人が占い師COしていた場合
            if (strcmp((persons+i)->say_position , "seer") == 0)
            {
                // 占い師と確定していない場合のみ
                if (strcmp((persons+i)->position , "seer") != 0)
                {
                    (persons+i)->position = "seer";
                    minus_remain((persons+i)->position, (persons+i)->say_position, remains);
                    int person_number = i;
                    true_seer(person_count, person_number, persons, remains);
                }
            }
            else
            {
                if (strcmp((persons+i)->position , "villager") != 0)
                {
                    (persons+i)->position = "villager";
                    minus_remain((persons+i)->position, (persons+i)->say_position, remains);
                }
            }
        }
    }
}


// 占い師の真偽がすべてついていなくても、人狼の配分（占い師騙りの人数と村人潜伏の人数）
// 部屋の役職が確定していない総人数と各役職の余りから推理
void check_remain_no_seer(int person_count, person *persons, remain *remains)
{
    // 人狼の村人潜伏がいない場合、村人はすべて真村人
    int seer_werewolf = remains->say_seer - remains->seer;
    if (seer_werewolf - remains->werewolf == 0)
    {
        for(int i = 0; i < person_count; i++)
        {
            if (strcmp((persons+i)->position, "(unclear)") == 0)
            {
                (persons+i)->position = "villager";
                minus_remain((persons+i)->position,(persons+i)->say_position, remains);
            }
        }
    }
    else if (remains->villager == 0)
    {
        for(int i = 0; i < person_count; i++)
        {
            if (strcmp((persons+i)->position, "(unclear)") == 0)
            {
                (persons+i)->position = "werewolf";
                minus_remain((persons+i)->position,(persons+i)->say_position, remains);
            }
        }
    }
}


void print_remain(remain *remains)
{
    printf("職業が確定していない人数は%i\n", remains->all);
    printf("残った占い師の人数は%i\n", remains->seer);
    printf("残った人狼の人数は%i\n", remains->werewolf);
    printf("残った村人の人数は%i\n", remains->villager);
    printf("残った確定していない占い師COの人数は%i\n", remains->say_seer);
}


void print_output(int person_count, person *persons)
{
    for (int i = 0; i < person_count; i++)
    {
        printf("%sは%sである\n", (persons+i)->name, (persons+i)->position);
    }
}