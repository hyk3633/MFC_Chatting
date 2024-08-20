# C++ MFC 채팅 프로그램
## 💻 프로젝트 소개
C++과 MFC로 개발한 채팅 프로그램 입니다.
## 🗓 개발 기간
24.08.12 ~ 24.08.20
## 🖥 개발 환경 및 사용 언어
>Visual Studio 2022

>C++

## ⚙ 주요 클래스 소개
### Client - [자세히 보기](https://github.com/hyk3633/MFC_Chatting/wiki/Client-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 서버 연결 및 데이터 송수신
- 다양한 패킷 처리 및 다이얼로그와 서버 사이의 데이터 중계

### MFC_ClientDlg - [자세히 보기](https://github.com/hyk3633/MFC_Chatting/wiki/MFC_ClientDlg-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 하위 다이얼 로그 생성
- 클라이언트의 메시지 처리
- 대화 내용 저장 및 불러오기
- 프로필 사진 설정

### ClientDlg_Main - [자세히 보기](https://github.com/hyk3633/MFC_Chatting/wiki/ClientDlg_Main-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 에딧 컨트롤에 입력된 문자열 전달
- 프로필 이미지 설정
- 리스트 컨트롤에서 클릭한 클라이언트의 프로필 이미지 창 생성

### Server - [자세히 보기](https://github.com/hyk3633/MFC_Chatting/wiki/Server-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 클라이언트와 데이터 송수신 및 브로드캐스팅
- 클라이언트의 채팅 메시지 브로드캐스팅
- 이미지 파일 수신, 저장 및 브로드캐스팅
- 클라이언트가 요청한 이미지 파일 찾아서 전달
- 연결 종료된 클라이언트 아이디 삭제 및 브로드캐스팅
- 서버 종료 시 자원 반환

### MFC_ServerDlg - [자세히 보기](https://github.com/hyk3633/MFC_Chatting/wiki/MFC_ServerDlg-%ED%81%B4%EB%9E%98%EC%8A%A4-%EC%86%8C%EA%B0%9C)
- 접속한 클라이언트의 아이디 추가 및 접속 종료한 클라이언트의 아이디 제거
