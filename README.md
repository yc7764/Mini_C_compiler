# Mini_C_compiler
특정 프로그래밍 언어로 쓰인 문서를 다른 프로그래밍 언어로 옮기는 언어 번역 프로그램을 컴파일러라고 하며, C언어의 문법을 간소화한 Mini C언어를 Ucode로 번역하는 Mini C compiler를 제작
Mini C언어의 문법으로 작성된 코드를 중간언어인 Ucode를 사용해 번역하여 중간코드를 생성한다. 생성된 중간코드는 Ucode interpreter를 사용하여 실행이 가능하다.  

## 프로그램 구성도
프로그램은 크게 세가지 모듈로 작성되었으며, 어휘 분석기, 구문 분석기, 중간코드 생성기로 구성된다.
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
- 구문 분석 과정
- 파스트리  
![image](https://user-images.githubusercontent.com/59434021/125738564-fea9a488-47df-4f2f-9b04-a89be407aaa2.png)  
- AST  
![image](https://user-images.githubusercontent.com/59434021/125738690-d156e72c-a3b1-4bff-a8ff-1d981db20965.png)  
### 3. 중간코드 생성기(Intermediate code generater)
- 구문 분석기에서 생성된 AST의 root노드 부터 모든 노드를 순회하며 각 문장의 의미에 맞는 Ucode를 출력
