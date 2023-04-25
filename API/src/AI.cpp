#include <vector>
#include <thread>
#include <array>
#include <math.h>
#include "AI.h"
#include "constants.h"
#define PI 3.14159265358979323846

// 为假则play()期间确保游戏状态不更新，为真则只保证游戏状态在调用相关方法时不更新
extern const bool asynchronous = false;
// 选手需要依次将player0到player4的职业在这里定义

extern const std::array<THUAI6::StudentType,4> studentType ={
    THUAI6::StudentType::StraightAStudent,
    THUAI6::StudentType::Teacher,
    THUAI6::StudentType::Athlete,
    THUAI6::StudentType::Sunshine};

extern const THUAI6::TrickerType trickerType = THUAI6::TrickerType::Assassin;

// 可以在AI.cpp内部声明变量与函数

// 前进方向
extern double directionS[4] ={0, 0, 0, 0};

// 返回目的地与现在位置间的角度，目的地为格数，现在位置为坐标
double angleangleInRadian(int32_t add[2],int add0[2])
{
    int addx = 1000 * add0[0] + 500;
    int addy = 1000 * add0[1] + 500;
    double a;
    if((addx - add[0]) == 0)
    {
        if(addy >= add[1])
            a = PI / 2;
        else
            a = 3 * PI / 2;
    }
    else
    {
        double tan = (double)(addy - add[1]) / (double)(addx - add[0]);
        if(addx > add[0])
            a = atan(tan);
        else
            a = atan(tan) + PI;
    }
    return a;
}

// 判断目的地与现在位置间的距离，目的地为格数，现在位置为坐标
int32_t Distance(int32_t add[2],int joint[2])
{
    int jointx = 1000 * joint[0] + 500;
    int jointy = 1000 * joint[1] + 500;
    return sqrt(pow(jointx - add[0],2) + pow(jointy - add[1],2));
}

// 判断目标点是否可视  (for student)
bool CheckVisible(IStudentAPI &api,int32_t add[2],int joint[2])
{
    double angle = angleangleInRadian(add,joint);
    double dis;
    bool flag = true;
    int32_t check[2];
    check[0] = add[0];
    check[1] = add[1];
    dis = Distance(add,joint);
    while(Distance(check,joint) > 750)
    {
        if(api.GetPlaceType(api.GridToCell(check[0]),api.GridToCell(check[1])) == (enum THUAI6::PlaceType)2)
        {
            flag = false;
            break;
        }
        check[0] += 250 * cos(angle);
        check[1] += 250 * sin(angle);
    }
    return flag;
}

// 判断目标点是否可视  (for tricker)
bool CheckVisible(ITrickerAPI &api,int32_t add[2],int joint[2])
{
    double angle = angleangleInRadian(add,joint);
    double dis;
    bool flag = true;
    int32_t check[2];
    check[0] = add[0];
    check[1] = add[1];
    dis = Distance(add,joint);
    while(Distance(check,joint) > 750)
    {
        if(api.GetPlaceType(api.GridToCell(check[0]),api.GridToCell(check[1])) == (enum THUAI6::PlaceType)2)
        {
            flag = false;
            break;
        }
        check[0] += 250 * cos(angle);
        check[1] += 250 * sin(angle);
    }
    return flag;
}

// 判断目标点是否可到达 (for student)
bool CheckApproachable(IStudentAPI &api,int32_t add[2],int joint[2])
{
    int edge[2];
    bool flag = true;
    double dir = angleangleInRadian(add,joint);
    for(int i = 0; i <= 1; i++)
    {
        edge[0] = add[0] + 600 * cos(dir + PI / 2 - i * PI);
        edge[1] = add[1] + 600 * sin(dir + PI / 2 - i * PI);
        flag = flag && CheckVisible(api,edge,joint);
    }
    return flag;
}

// 判断目标点是否可到达 (for tricker)
bool CheckApproachable(ITrickerAPI &api,int32_t add[2],int joint[2])
{
    int edge[2];
    bool flag = true;
    double dir = angleangleInRadian(add,joint);
    for(int i = 0; i <= 1; i++)
    {
        edge[0] = add[0] + 600 * cos(dir + PI / 2 - i * PI);
        edge[1] = add[1] + 600 * sin(dir + PI / 2 - i * PI);
        flag = flag && CheckVisible(api,edge,joint);
    }
    return flag;
}

// 判断前进的方向
double Direction(int32_t add[2],int joint[2])
{
    double ang = angleangleInRadian(add,joint);
    double dir = 0;
    for(int i = 1; i <= 29; i += 2)
    {
        if(ang > (double)i * PI / 16 && ang <= (double)(i + 2) * PI / 16)
            dir = (double)(i + 1) * PI / 16;
    }
    return dir;
}

// 检查前进方向是否有阻碍
bool CheckBlocekd(IStudentAPI &api,int32_t add[2],double dir)
{
    int edge[2];
    bool flag = false;
    for(int i = 0; i <= 2; i++)
    {
        edge[0] = add[0] + 600 * cos(dir + PI / 2 - i * PI / 2);
        edge[1] = add[1] + 600 * sin(dir + PI / 2 - i * PI / 2);
        for(int32_t dis = 250; dis <= 1000; dis += 250)
        {
            int front[2] ={(edge[0] + dis * cos(dir)) / 1000, (edge[1] + dis * sin(dir)) / 1000};
            if(api.GetPlaceType(front[0],front[1]) == (enum THUAI6::PlaceType)2 || api.GetPlaceType(front[0],front[1]) == (enum THUAI6::PlaceType)4)
            {
                flag = true;
                break;
            }
        }
    }
    return flag;
}

// 判断被阻碍时应向左转还是向右转,逆时针为正
double Turn(IStudentAPI &api,int32_t add[2],double dir)
{
    double turn;
    int d[9];
    bool di[9];
    for(int i = 0; i < 9; i++)
    {
        d[i] = 0;
        di[i] = true;
    }
    bool flag = true;
    int max = 0;
    for(int32_t dis = 500; flag; dis += 500)
    {
        for(int i = 0; i < 9; i++)
        {
            if(i == 4)
                continue;
            if(di[i])
            {
                int x = (add[0] + dis * cos(dir + PI / 2 - i * PI / 8)) / 1000;
                int y = (add[1] + dis * sin(dir + PI / 2 - i * PI / 8)) / 1000;
                if((api.GetPlaceType(x,y) == (enum THUAI6::PlaceType)2) || (api.GetPlaceType(x,y) == (enum THUAI6::PlaceType)4))
                {
                    di[i] = false;
                }
                else
                {
                    d[i]++;
                }
            }
            flag = flag || di[i];
        }
        flag = flag && (dis < 12000);
    }
    for(int i = 0; i < 9; i++)
    {
        if(i == 4)
            continue;
        if(d[i] > max)
        {
            turn = (4 - i) * PI / 8;
            max = d[i];
        }
    }
    return turn;
}

// 判断目标点与现在位置是否在同一个九宫格内
bool CheckNine(int32_t add[2],int joint[2])
{
    int addCell[2] ={add[0] / 1000, add[1] / 1000};
    bool flag = false;
    if((abs(addCell[0] - joint[0]) < 1.5) || (abs(addCell[1] - joint[1]) < 1.5))
        flag = true;
    return flag;
}

// 判断门开的方向，竖直开（水平两边是墙）返回0，水平开返回1
int DoorDirection(IStudentAPI &api,int joint[2])
{
    int dir = 0;
    if(api.GetPlaceType(joint[0] + 1,joint[1]) == (enum THUAI6::PlaceType)2)
        dir = 1;
    return dir;
}

void AI::play(IStudentAPI &api)
{
    int id = this->playerID;
    int32_t address[2] ={api.GetSelfInfo()->x, api.GetSelfInfo()->y};                                      // 获取自身坐标
    int addressCell[2] ={api.GridToCell(api.GetSelfInfo()->x), api.GridToCell(api.GetSelfInfo()->y)};      // 获取自身所在格数
    int ClassroomVisible[3] ={0, 0, 0};                                                                    // 可见的教室格子坐标,第三位表示是否找到
    int DoorVisible[4] ={0, 0, 0, 0};                                                                      // 可见的门坐标，第三位表示门开的方向，0为竖直开，1为水平开；第四位表示是否找到
    for(int i = (addressCell[0] >= 12) ? addressCell[0] - 12 : 0; i <= addressCell[0] + 12 && i < 50; i++) // 扫描周围24方内的地点
    {
        for(int j = (addressCell[1] >= 12) ? addressCell[1] - 12 : 0; j <= addressCell[1] + 12 && j < 50; j++)
        {
            int joint[2] ={i, j};
            if((api.GetPlaceType(i,j) == ((enum THUAI6::PlaceType)4)) && (ClassroomVisible[2] == 0) && CheckApproachable(api,address,joint) && (api.GetClassroomProgress(i,j) < 10000000))
            {
                ClassroomVisible[0] = i;
                ClassroomVisible[1] = j;
                ClassroomVisible[2] = 1;
            }
            if((api.GetPlaceType(i,j) == ((enum THUAI6::PlaceType)8) || api.GetPlaceType(i,j) == ((enum THUAI6::PlaceType)9) || api.GetPlaceType(i,j) == ((enum THUAI6::PlaceType)10)) && (api.GetDoorProgress(i,j) == 10000000) && (DoorVisible[3] == 0))
            {
                DoorVisible[0] = i;
                DoorVisible[1] = j;
                DoorVisible[2] = DoorDirection(api,joint);
                DoorVisible[3] = 1;
            }
        }
    }
    if(ClassroomVisible[2] == 1) // 若视野内有可见教室，向其移动，到达时开始学习
    {
        api.Move(50,angleangleInRadian(address,ClassroomVisible));
        directionS[id] = Direction(address,ClassroomVisible);
        if(CheckNine(address,ClassroomVisible))
            api.StartLearning();
    }
    /*else if (DoorVisible[3] == 1) // 若视野内无教室但有门
    {
        int delta = (address[DoorVisible[2]] / 1000 > DoorVisible[DoorVisible[2]]);
    }*/
    else
    {
        if(CheckBlocekd(api,address,directionS[id]) == false)
            api.Move(50,directionS[id]);
        else
        {
            directionS[id] += Turn(api,address,directionS[id]);
            api.Move(50,directionS[id]);
        }
    }
    // 公共操作
    if(this->playerID == 0)
    {
        // 玩家0执行操作
    }
    else if(this->playerID == 1)
    {
        // 玩家1执行操作
    }
    else if(this->playerID == 2)
    {
        // 玩家2执行操作
    }
    else if(this->playerID == 3)
    {
        // 玩家3执行操作
    }
    // 当然可以写成if (this->playerID == 2||this->playerID == 3)之类的操作
    //  公共操作
}

void AI::play(ITrickerAPI &api)
{
    auto self = api.GetSelfInfo();
    api.PrintSelfInfo();

    int32_t address[2] ={api.GetSelfInfo()->x, api.GetSelfInfo()->y};                                      // 获取自身坐标
    int addressCell[2] ={api.GridToCell(api.GetSelfInfo()->x), api.GridToCell(api.GetSelfInfo()->y)};      // 获取自身所在格数
    int studentInView[5][2] ={{0,0},{0,0},{0,0},{0,0},{0,0}};//视野内有无学生，若有，返回格子坐标；第一个指标为学生序号，第二个指标为坐标
    int studentInViewCell[5][2] ={{0,0},{0,0},{0,0},{0,0},{0,0}};//视野内有无学生(格子版)
    std::vector<std::shared_ptr<const THUAI6::Student>> student = api.GetStudents(); //获取视野可见学生信息
    if((int)student.size()>0)//如果视野内有学生
    {
        for(int i=0;i<(int)student.size();i++)
        {
            studentInView[i][0]=(*student[i]).x;
            studentInView[i][1]=(*student[i]).y;
            studentInViewCell[i][0]=api.GridToCell((*student[i]).x);
            studentInViewCell[i][1]=api.GridToCell((*student[i]).y);
        }

        api.Move(50,angleangleInRadian(address,studentInViewCell[0]));
    }
    else
    {
        //api.Move(100,5);
        /* bool move=1;
         if(move)
         {
             api.MoveLeft(500);
             move=false;
         }
         else
         {
             api.MoveRight(100);
             move=true;
         }*/
    }

    //进行攻击
    if(CheckNine(address,studentInViewCell[0]))
    {
        api.Attack(angleangleInRadian(address,studentInViewCell[0]));
    }
}

/*std::vector < std::vector < THUAI6::PlaceType >> map = api.GetFullMap();
    std::vector<int*> Land(1,Zero), Wall(1, Zero), Grass(1, Zero), Classroom(1, Zero), Gate(1, Zero), HiddenGate(1, Zero), Window(1, Zero), Door3(1, Zero), Door5(1, Zero), Door6(1, Zero), Chest(1, Zero);
    for (int i = 0; i <= 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            int point[2] = { i,j };
            switch (map[i][j])
            {
            case (enum THUAI6::PlaceType)1:Land.emplace_back(point); break;
            case (enum THUAI6::PlaceType)2:Wall.emplace_back(point); break;
            case (enum THUAI6::PlaceType)3:Grass.emplace_back(point); break;
            case (enum THUAI6::PlaceType)4:Classroom.emplace_back(point); break;
            case (enum THUAI6::PlaceType)5:Gate.emplace_back(point); break;
            case (enum THUAI6::PlaceType)6:HiddenGate.emplace_back(point); break;
            case (enum THUAI6::PlaceType)7:Window.emplace_back(point); break;
            case (enum THUAI6::PlaceType)8:Door3.emplace_back(point); break;
            case (enum THUAI6::PlaceType)9:Door5.emplace_back(point); break;
            case (enum THUAI6::PlaceType)10:Door6.emplace_back(point); break;
            case (enum THUAI6::PlaceType)11:Chest.emplace_back(point); break;
            }
        }
    }
    for (int i = 0; i<Classroom.size(); i++)
    {
        if (distance(address, Classroom[i]) < 12000)
        {
            if(CheckVisible(api,address,Classroom[i]))
            {
                ClassroomVisible[0] = Classroom[i][0];
                ClassroomVisible[1] = Classroom[i][1];
                break;
            }
        }

    }*/