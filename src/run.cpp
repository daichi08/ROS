#include <ros/ros.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

struct TurningPoint {
  double x;
  double y;
};

int main(int argc, char** argv){
  TurningPoint tp[] = {{-1.5,-1.5},{1.5,-1.5},{1.5,1.5},{-1.5,1.5},{-2.0,-0.5}};

  ros::Publisher  pub;

  ros::init(argc, argv, "simple_goal");

  ros::NodeHandle nh;
  pub= nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("initialpose", 2, true);

  // 時間の取得
  ros::Time tmp_time = ros::Time::now();
  //create msg
  geometry_msgs::PoseWithCovarianceStamped initPose;

  // 初期位置の設定 turtlebot3_worldの初期位置が(-2.0, -0.5)なのでそれに設定
  initPose.header.stamp = tmp_time;  //  時間
  initPose.header.frame_id = "map";  //  フレーム
  initPose.pose.pose.position.x = -2.0;
  initPose.pose.pose.position.y = -0.5;
  initPose.pose.pose.position.z =  0;
  initPose.pose.pose.orientation.w = 1.0;

  // パブリッシュ amclパッケージに初期位置を送る
  pub.publish(initPose);
  
  double current_x = initPose.pose.pose.position.x;
  double current_y = initPose.pose.pose.position.y;
  
  // アクションクライアンを作成。1番目の引数は接続するアクションサーバー名。
  // アクションサーバーが立ち上がっていないとだめ。
  // ２番目の引数はtrueならスレッドを自動的に回す(ros::spin()。
  MoveBaseClient ac("move_base", true);
  // アクションサーバーが起動するまで待つ。引数はタイムアウトする時間(秒）。
  // この例では５秒間待つ(ブロックされる)
  while(!ac.waitForServer(ros::Duration(5.0))){
      ROS_INFO("Waiting for the move_base action server to come up");
  }

  ROS_INFO("The server comes up");
  move_base_msgs::MoveBaseGoal goal;
  // base_link座標系（ロボット座標系)
  goal.target_pose.header.frame_id = "base_link";
  // 現在時刻                                                                       
  goal.target_pose.header.stamp = ros::Time::now();


  int i = 0;
  int t_count = 0;
  
  while (ros::ok()) {
    if(current_x == tp[t_count].x && current_y == tp[t_count].y){
      t_count += 1;
    }
    
    if(current_x < tp[t_count].x){
      current_x += 0.1;
    }else if(current_x > tp[t_count].x){
      current_x -= 0.1;
    }else{
      current_x += 0.0;
    }
    
    if(current_y < tp[t_count].y){
      current_y += 0.1;
    }else if(current_y > tp[t_count].y){
      current_y -= 0.1;
    }else{
      current_y += 0.0;
    }
    
    if(t_count > 5){
      current_x = 999;
      current_y = 999;
    }
    // ROSではロボットの進行方向がx座標、左方向がy座標、上方向がz座標
    goal.target_pose.pose.position.x = current_x;
    goal.target_pose.pose.position.y = current_y;
    goal.target_pose.pose.orientation.w = 1.0;

    ROS_INFO("Sending goal: No.%d", i+1);
    // サーバーにgoalを送信
    ac.sendGoal(goal);

    // 結果が返ってくるまで30.0[s] 待つ。ここでブロックされる。
    bool succeeded = ac.waitForResult(ros::Duration(30.0));

    // 結果を見て、成功ならSucceeded、失敗ならFailedと表示
    actionlib::SimpleClientGoalState state = ac.getState();

    if(succeeded) {
      ROS_INFO("Succeeded: No.%d (%s)",i+1, state.toString().c_str());
    }
    else {
      ROS_INFO("Failed: No.%d (%s)",i+1, state.toString().c_str());
    }
    i++;  
  }
  return 0;
}

