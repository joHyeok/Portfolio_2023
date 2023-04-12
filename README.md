# Portfolio_2023
포트폴리오 기술용입니다.   
여러 프로젝트의 일부 코드를 옮겨온 코드입니다.   
그렇기에 내려 받아 컴파일 시 에러가 나는 제대로 동작하지 않은 코드들이고 오로지 포트폴리오용으로 쓰입니다.   
   
사용한 블루프린트들은 여기서 볼 수 있습니다. [Show Blueprint](https://blueprintue.com/profile/mandu/)   
## 바나랜드
![소켓 매니저](https://user-images.githubusercontent.com/69950874/230727505-ce034f74-b61f-4602-8564-060b400bba2a.PNG)   
![Table Chair Class](https://user-images.githubusercontent.com/69950874/230727838-d49fda09-a201-4464-ab95-bca6ce1c1882.PNG)   
   
* 소켓 매니저는 레벨에 배치되어 있는 6개의 TableChairClass액터를 가지고 있으며   
TableChairClass는 각각 프로필, 카드, 코인 액터를 가지고 있습니다.   
* 서버로부터 받은 이벤트에 따라 각 액터에 접근해 동작을 수행합니다.  
[Source/Portfolio_2023/Main/SocketManagerBase.cpp에서 확인해 볼 수 있습니다.](Source/Portfolio_2023/Main/SocketManagerBase.cpp)   
   
## 버터랜드
* 앱 실행 이후 추가 다운로드 기능 구현을 위해 언리얼에서 데이터 에셋을 사용한 pak 파일 생성   
* aws S3에 pak파일을 업로드, 앱에서 다운로드 기능 구현   
[Source/Portfolio_2023/Main/DownloadPakBase.cpp에서 확인해 볼 수 있습니다.](Source/Portfolio_2023/Main/DownloadPakBase.cpp)   
   
## 이멀전
![image](https://user-images.githubusercontent.com/69950874/231374279-ae2a8b22-daaf-45bd-90c4-94ec535b90be.png)   
   
* 두개의 스플라인 컴포넌트   
* 러프하게 스플라인을 잡으면 박스 트레이스를 사용해 랜드스케이프의 굴곡에 맞춰서 100cm 간격의 촘촘한 스플라인 생성   
* 스플라인 패스상에 충돌 가능한 액터가 있다면 빨간선으로 표시   
[Source/Portfolio_2023/Main/SplineDirBase.cpp에서 확인해 볼 수 있습니다.](Source/Portfolio_2023/Main/SplineDirBase.cpp)   
   
## 재난 상황 VR
* VR 컨트롤러와 언리얼상 소방 호스의 위치, 회전값을 서버, 클라이언트 연동 시키면 각 로컬 pc에 연결된 VR기기와 바인딩된 소방 호스 탓인지 에러가 난다.   
* 더미 핸드, 더미 헤드와 같이 VR 컨트롤러와 HMD의 회전값과 연동할 더미 컴포넌트를 만들고 더미의 데이터를 서버와 연동 시키는 방식으로 해결하였다.   
[Source/Portfolio_2023/Main/DummyHand.cpp에서 확인할 수 있다.](Source/Portfolio_2023/Main/DummyHand.cpp)
