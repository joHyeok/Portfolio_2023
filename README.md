# Portfolio_2023
포트폴리오 기술용입니다.   
여러 프로젝트의 일부 코드를 옮겨온 코드입니다.   
그렇기에 내려 받아 컴파일 시 에러가 나는 제대로 동작하지 않은 코드들이고 오로지 포트폴리오용으로 쓰입니다.   
   
사용한 블루프린트들은 여기서 볼 수 있습니다. [Show Blueprint](https://blueprintue.com/profile/mandu/)   
## 바나랜드
![소켓 매니저](https://user-images.githubusercontent.com/69950874/230727505-ce034f74-b61f-4602-8564-060b400bba2a.PNG)   
![Table Chair Class](https://user-images.githubusercontent.com/69950874/230727838-d49fda09-a201-4464-ab95-bca6ce1c1882.PNG)   
   
소켓 매니저는 레벨에 배치되어 있는 6개의 TableChairClass액터를 가지고 있으며 TableChairClass는 각각 프로필, 카드, 코인 액터를 가지고 있습니다.   
서버로부터 받은 이벤트에 따라 각 액터에 접근해 동작을 수행합니다.  
[Source/Portfolio_2023/Main/SocketManagerBase.cpp에서 확인해 볼 수 있습니다.](Source/Portfolio_2023/Main/SocketManagerBase.cpp)   
