# Mini_C_compiler
특정 프로그래밍 언어로 쓰인 문서를 다른 프로그래밍 언어로 옮기는 언어 번역 프로그램을 컴파일러라고 하며, C언어의 문법을 간소화한 Mini C언어를 Ucode로 번역하는 Mini C compiler를 제작
Mini C언어의 문법으로 작성된 코드를 중간언어인 Ucode를 사용해 번역하여 중간코드를 생성한다. 생성된 중간코드는 Ucode interpreter를 사용하여 실행이 가능하다.  

## 프로그램 구성도
프로그램은 크게 세가지 모듈로 작성되었으며, 어휘 분석기, 구문 분석기, 중간코드 생성기로 구성된다. scanner() 함수가 처음으로 호출되면 코드의 첫 토큰을 반환하게 되고, 이후 함수 호출 시에는 그 다음 토큰을 반환하여 첫 토큰부터 마지막 토큰까지 구분하게 된다. parser() 함수는 scanner()함수에서 반환된 토큰들을 구문 분석하여 토큰 별로 노드를 생성하고 트리를 구성하여 트리의 루트 노드를 반환한다. scanner() 함수를 반복하여 호출하며 마지막 토큰이 나올 떄까지 반복하게 된다. 구문 분석 시 MiniC.tbl 파일의 파싱 테이블을 이용하여 문법에 맞게 토큰들을 구분하고 구문 오류를 찾아낸다.
마지막으로 codeGen() 함수를 호출하여 루트노드부터 모든 노드를 순회하면 Ucode를 생성하게 된다. 이때 필요한 심볼들의 이름과 데이터를 관리하기 위하여 SymTab.c 파일의 심볼 테이블을 사용하였다.
변수 또는 함수의 선언인 노드에서는 심볼 테이블에 심볼의 정보를 저장하고, 변수의 사용 또는 함수 호출 시 심볼 테이블에서 해당 심볼의 데이터를 검색하여 사용한다.
### 1.어휘 분석기(Lexical analyzer)
- 최소한의 의미를 가지는 단어를 토큰이라고 하며, 입력 코드를 토큰별로 나누는 모듈  
- 만약 입력 코드가 아래와 같다면 if, (, a, >, 10, ) 총 6개의 토큰으로 나누어 진다.
  ```sh
  if ( a > 10 )
  ```
### 2. 구문 분석기(Syntax anlyzer)
- 올바른 문장을 트리 형식으로 나타낸 파스트리(parse tree)라고 한다.
- 실제 구문에서 나타나는 세세한 정보를 나타내지 않고 필수적인 정보를 나타낸 트리를 추상구문트리(Abstract Syntax Tree) 또는 AST라고 한다.
- 어휘 분석기에 의해 나눠진 일련의 토큰들을 Mini C 문법에 맞는 파스트리와 AST를 생성한다.
- 문법에 맞지않는 문장이 입력될 경우 에러메시지를 출력
- 구문 분석기는 Shift-reduce 구문 분석방법을 사용하며, input에서 한 토큰을 stack으로 옮기는 걸 shift라고 하며, 이때 단말노드가 생성된다. 생성규칙에 의해서 stack의 데이터가 nonterminal로 유도되는 것을 reduce라고 하며, 이때 각 단말 노드가 서브트리로 이어진다. 만약 생성규칙과 input이 아래와 같다면 구문분석과정은 표와 같고 아래의 사진처럼 파스트리와 AST가 생성된다.
- 생성규칙
  - E -> E - T  =>sub
  - E -> T
  - T -> T * F  =>mul
  - T -> F
  - F -> &P     =>addr
  - F -> P
  - P -> (E)
  - P -> id
- input
```sh
id*id-&id
```
구문 분석 과정
| Step | Stack  | Input      | Action   | Parse Tree      | AST             |
|------|--------|------------|----------|-----------------|-----------------|
| 1    | $      | id*id-&id$ | Shift id | Build Node (id) | Build Node (id) |
| 2    | $id    | *id-&id$   | Reduce 8 | Build Tree (8)  |                 |
| 3    | $P     | *id-&id$   | Reduce 6 | Build Tree (6)  |                 |
| 4    | $F     | *id-&id$   | Reduce 4 | Build Tree (4)  |                 |
| 5    | $T     | *id-&id$   | Shift *  | Build Node (*)  |                 |
| 6    | $T*    | id-&id$    | Shift id | Build Node (id) | Build Node (id) |
| 7    | $T*id  | -&id$      | Reduce 8 | Build Tree (8)  |                 |
| 8    | $T*P   | -&id$      | Reduce 6 | Build Tree (6)  |                 |
| 9    | $T*F   | -&id$      | Reduce 3 | Build Tree (3)  | Build Tree (3)  |
| 10   | $T     | -&id$      | Reduce 2 | Build Tree (2)  |                 |
| 11   | $E     | -&id$      | Shift -  | Build Node (-)  |                 |
| 12   | $E-    | &id$       | Shift &  | Build Node (&)  |                 |
| 13   | $E-&   | id$        | Shift id | Build Node (id) | Build Node (id) |
| 14   | $E-&id | $          | Reduce 8 | Build Tree (8)  |                 |
| 15   | $E-&P  | $          | Reduce 5 | Build Tree (5)  | Build Tree (5)  |
| 16   | $E-F   | $          | Reduce 4 | Build Tree (4)  |                 |
| 17   | $E-T   | $          | Reduce 1 | Build Tree (1)  | Build Tree (1)  |
| 18   | $E     | $          | accept   | Return Tree     | Return Tree     |
- 파스트리  
![image](https://user-images.githubusercontent.com/59434021/125738564-fea9a488-47df-4f2f-9b04-a89be407aaa2.png)  
- AST  
![image](https://user-images.githubusercontent.com/59434021/125738690-d156e72c-a3b1-4bff-a8ff-1d981db20965.png)  
### 3. 중간코드 생성기(Intermediate code generater)
- 구문 분석기에서 생성된 AST의 root노드 부터 모든 노드를 순회하며 각 문장의 의미에 맞는 Ucode를 출력
- Ucode
  - Ucode 명령어는 아래와 같이 총 39개로 구성
    - 단항 명령어 : notop, neg, inc, dec, dup
    - 이항 명령어 : add, sub, mult, div, mod, swp, and, or, gt, lt, ge, le, eq, ne
    - 스택 연산 명령어 : lod, str, ldc, lda
    - 제어 흐름 명령어 : ujp, tjp, fjp
    - 범위 체크 명령어 : chkh, chkl
    - 주소 참조 명령어 : ldi(load indirect), sti(store indirect)
    - 프로시저 명령어 : call, ret, retv, ldp, proc, end
    - 기타 : nop, bgn, sym
### 4. 생성된 중간코드 실행법
 1. Mini C 문법에 맞는 코드를 작성하여 입력 파일인 mc파일 생성하고 Debug 폴더의 MiniC_Compiler.exe와 ucodei.exe를 확인
    -perfect.mc 파일 작성
    ```sh
    const int max = 500;
    void main(){
      int i, j, k;
      int rem, sum;
      i = 2;
      while (i <= max){
        sum = 0;
        k = i / 2;
        j = 1;
        while (j <= k){
          rem = i % j;
          if(rem == 0) sum += j;
          ++j;
        }
        if (i == sum) write(i);
        ++i;
      }
    }
    ```
 2. 실행창 실행 후 Debug 폴더에서 명령어 MiniC_Compiler “출력파일이름” < ”입력파일이름.mc” 실행
    ```sh
    MiniC_Compiler perfect < perfect.mc
    ```
 3. 생성된 uco 파일을 확인한 후 명령어 ucodei 출력파일이름.uco out.lst 실행
    ```sh
    ucodei perfect.uco out.lst
    ```
    -실행결과  
    ![image](https://user-images.githubusercontent.com/59434021/125977633-e4ba88b0-13e4-4cb1-a15f-02bb75eb21cb.png)
