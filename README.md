# Portfolio_2023
포트폴리오 기술용입니다.   
여러 프로젝트의 일부 코드를 옮겨온 코드입니다.   
그렇기에 내려 받아 컴파일 시 에러가 나는 제대로 동작하지 않은 코드들이고 오로지 포트폴리오용으로 쓰입니다.   
이 프로젝트의 코드를 복사하여 사용시 발생하는 버그에 대해 책임지지 않습니다.   
   
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
