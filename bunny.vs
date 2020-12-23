#version 330 core
layout (location = 0) in vec3 aPos;		//顶点位置属性 0
layout (location = 1) in vec3 aNormal;	//顶点法线属性 1

out vec3 FragPos;	
out vec3 Normal;

uniform mat4 model; //转换成世界空间坐标系所需要的模型矩阵。
uniform mat4 view; //转换成观察者空间坐标系所需要的观察矩阵。
uniform mat4 projection; //转换成裁剪空间坐标系所需要的裁剪矩阵（投影矩阵）

void main()
{
	//在世界空间坐标系下的各个片段的位置向量
    FragPos = vec3(model * vec4(aPos, 1.0));
	//通过正规矩阵处理的法向量，inverse函数是求逆函数，transpose是转置函数。
    Normal = mat3(transpose(inverse(model))) * aNormal;  
	//记录最后各个顶点的最终位置向量
    gl_Position = projection * view * vec4(FragPos, 1.0);
}