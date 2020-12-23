#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include "parser.h"

#define INF 1.0e10
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_click_callback(GLFWwindow *window, int button, int action, int mods);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void do_movement();

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 750;

//初始化相机的位置
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

//是否为第一次进入程序
bool  firstMouse = true;

// 时间参数（缓冲渲染两帧之差 的时间）
float deltaTime = 0.0f;  //当前帧与上一帧的时间差
float lastFrame = 0.0f;  //上一帧的时间

// 兔子参数
glm::mat4 modelBunny = glm::mat4(1.0f);// 四阶单位阵
GLuint vertices_size, indices_size;	     // 顶点个数以及面片个数

// 光照参数(声明一个全局vec3变量来表示光源在场景的世界空间坐标中的位置)
glm::vec3 PointlightPos(5.0f, 13.0f, 4.0f);	// 三维向量 光照

//鼠标参数（初始化光标的位置 屏幕中心）
GLfloat mouseX = SCR_WIDTH / 2.0;
GLfloat mouseY = SCR_HEIGHT / 2.0;

GLuint selectedPointIndice = 0; 

const int MAXPOINT = 40000; //.py文件 34834
const int MAXINDEX = 70000; //.py文件 69451

const char* DataFile = "bunny_normal.ply2";
const char *SPLIT = "--------------------------------------------------------------";
GLfloat vertices[MAXPOINT*6]; //顶点每行要有6个数据（x,y,z 法向量）
GLuint indices[MAXINDEX*3];   //面片每行有3个数据（顶点的索引号）

bool keys[1024];	//存取键码
bool isAttenuation = false;
bool isFlashlight = false;
bool cursorDisabled = true; //光标若禁用选不中

//按键说明
void KeyDescription()
{
    std::cout << "=: 点光源衰减\n";
    std::cout << "P: FlashLight开关\n";
    std::cout << "A: 相机向左.\n";
    std::cout << "D: 相机向右.\n";
    std::cout << "W: 相机向前\n";
    std::cout << "S: 相机向后.\n";
    std::cout << "方向键<-: 兔子左移.\n";
    std::cout << "方向键->: 兔子右移.\n";
    std::cout << "J: 兔子顺时针旋转.\n";
    std::cout << "L: 兔子逆时针旋转.\n";
    std::cout << "I: 兔子向前旋转.\n";
    std::cout << "K: 兔子向后旋转.\n";
    std::cout << "X: 兔子放大.\n";
    std::cout << "Z: 兔子缩小.\n";
    std::cout << "Tab: 线框模式（启用光标）| 默认模式（禁用光标）\n";
    std::cout << SPLIT << std::endl;
}

//窗口初始化函数
GLFWwindow* init(){
    //初始化glfw库
    glfwInit();
	
    //声明版本与核心
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//主版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //创建GLFW窗口并设置其大小，名称，与检测是否创建成功
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LONG_xy_Stanford bunny", NULL, NULL);
	
	//如果窗口创建失败，返回失败信息
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();	//释放所有资源
        exit(EXIT_SUCCESS);
    }
    //设置当前的窗口上下文
	/*创建完毕之后，需要让当前窗口的环境在当前线程上成为当前环境，
	 *就是接下来的绘制都会画在刚刚创建的窗口上*/
    glfwMakeContextCurrent(window);

	//当窗口调整大小的时候调用这个函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//注册按键回调函数
    glfwSetKeyCallback(window, key_callback);	

	//注册光标回调函数：监听鼠标移动事件 回调鼠标响应函数
    glfwSetCursorPosCallback(window, mouse_callback);	

    //注册鼠标按键回调函数
    glfwSetMouseButtonCallback(window, mouse_click_callback);
	
    //注册滚轮回调函数
    glfwSetScrollCallback(window, scroll_callback);		

    // 让GLFW捕获鼠标（隐藏鼠标的光标 光标会一直停留在窗口中）
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: 加载所有openGL函数指针（glad寻找opengl的函数地址，调用opengl的函数前需要初始化glad）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_SUCCESS);
    }
    // 配置opengl的全局状态
    /*启用深度测试，默认是关闭的
     *GLFW会自动生成Z-buffer 深度缓冲*/
    glEnable(GL_DEPTH_TEST);
    return window;
}

int main()
{
    KeyDescription();
    // glfw: 初始化并且配置相关参数
	//调用窗口初始化函数
    GLFWwindow* window = init();

    //读取着色器路径
    Shader lightingShader("bunny.vs", "bunny.fs");	// vertexShader fragmentShader
    Shader lampShader("lamp.vs", "lamp.fs");		//灯着色器
    Shader selectShader("select.vs", "select.fs");	//拾取着色器

    // 设置顶点数据（和缓冲区）并配置顶点属性
    //导入ply2文件
    parseNormal(DataFile, vertices, indices, vertices_size, indices_size);

   
	/*立方体顶点数据
	 *6个面 每个面2个三角形 一个三角形3个点 6*3*2  36个顶点*/
    float lightVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 
        -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 

        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, 

        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, 

         0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 

        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, 

        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
    };

    //第一步，配置兔子的VAO & VBO & EBO
    GLuint  bunnyVAO, bunnyVBO, bunnyEBO;
	
    glGenVertexArrays(1, &bunnyVAO);	//创建VAO对象
    glBindVertexArray(bunnyVAO);        //绑定VAO对象
	
    glGenBuffers(1, &bunnyVBO);         	    //创建VBO
    glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);    //绑定VBO
    glBufferData(GL_ARRAY_BUFFER, 6 * vertices_size * sizeof(GLfloat), vertices, GL_STATIC_DRAW);    //将用户定义的顶点数组复制到缓冲中供OpenGL使用
	
    glGenBuffers(1, &bunnyEBO);	                        //创建EBO索引缓冲对象        
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyEBO);	//绑定EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*indices_size*sizeof(GLuint), indices, GL_STATIC_DRAW);	  //传入EBO数据：将索引存储到EBO当中

    // 位置属性（设置顶点属性指针） 解释顶点数据方式
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);	//layout=0,vec3,连续顶点属性组间的间隔
	glEnableVertexAttribArray(0);	//开启顶点属性 （开启layout = 0，位置数据）
	
    // 法向量属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1); //开启layout = 1 法向数据

    glBindVertexArray(0); // 解绑VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // 解绑VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// 解绑EBO？


    // 其次，配置灯光的VAO & VBO
    GLuint lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);            	//绑定灯光VAO对象

    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);	//绑定灯光VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);	//灯光顶点数据绑定到缓存中
	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);	//通知OPENGL处理顶点数据	
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);             // 取消绑定VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // 取消绑定VBO
    
	//兔子的初始状态
    modelBunny = glm::rotate(modelBunny, glm::radians(145.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//沿y轴逆时针旋转145度
   
	//while渲染循环（程序在关闭它之前不断绘制图像并能够接受用户输入）
    while (!glfwWindowShouldClose(window)) //检查窗口是否需要关闭
    {	
        //计算上下两帧的渲染时间差
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // 输入
        do_movement();

    if (!cursorDisabled)//光标未禁用
    {
        glEnable(GL_CULL_FACE);                       //OpenGL的GL_CULL_FACE选项: 开启面剔除功能
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	//显示线段，多边形用轮廓显示
    }
    else  //光标被禁用 显示物体所有面,多边形采用填充式
      {
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    	
    // 渲染背景
    	/*在每个新的渲染迭代开始的时候我们总是希望清屏，否则仍能看见上一次迭代的渲染结果。
    	 *我们可以通过调用glClear函数来清空屏幕的颜色缓冲，
    	 *它接受一个缓冲位(Buffer Bit)来指定要清空的缓冲，
    	 */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//清除颜色缓冲以及深度缓冲

        // 设置格式/绘图对象前激活着色器
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // 灯光特性
        glm::vec3 lightColor;	
		
		//兔子颜色
        lightColor.x = 3.0f;
        lightColor.y = 0.0f;
        lightColor.z = 7.0f;
    	
        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.3f); 
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); 

        // 点光源
        lightingShader.setVec3("pointLights.position", PointlightPos);
        lightingShader.setVec3("pointLights.ambient", ambientColor);
        lightingShader.setVec3("pointLights.diffuse", diffuseColor);
        lightingShader.setVec3("pointLights.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights.constant", 1.0f);
        lightingShader.setFloat("pointLights.linear", (isAttenuation ? 0.015 : 0.0f));
        lightingShader.setFloat("pointLights.quadratic", (isAttenuation ? 0.0007 : 0.0f));
        
        // 聚光灯
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 5.0f, 5.0f, 5.0f);  
        lightingShader.setVec3("spotLight.diffuse", 5.0f, 5.0f, 5.0f);  
        lightingShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f); 
        lightingShader.setFloat("spotLight.constant", (isFlashlight? 1.0f : INF));
        lightingShader.setFloat("spotLight.linear", 0.045f);
        lightingShader.setFloat("spotLight.quadratic", 0.0075f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(5.0f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(9.0f)));     

        // 材料特性
        lightingShader.setVec3("material.ambient", 0.5f, 0.5f, 0.5f);	
        lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.2f);	
        lightingShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);  
        lightingShader.setFloat("material.shininess", 250.0f);

    	//View & Projection transformation
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    	//为着色器中的矩阵赋值
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", modelBunny); //兔子模型参数传入到model着色器

        // 渲染兔子
        glBindVertexArray(bunnyVAO);	//绑定兔子的VAO
        glDrawElements(GL_TRIANGLES, 3*indices_size, GL_UNSIGNED_INT, 0);//图元绘制函数 
        glBindVertexArray(0);	        //取消绑定VAO

        // 画出灯的对象
        lampShader.use();

        // 设置灯的相关矩阵
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, PointlightPos);       //把灯平移到点光源的位置，进行缩放，让它不那么明显
        model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube（立方体灯源）
        lampShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

        //拾取点的坐标
        if (selectedPointIndice <= vertices_size)  //如果点的索引小于点的个数
        {	
            selectShader.use();	//启用着色器
			/*创建一个四维向量记录当前点的位置
			 n1 n2 n3 分别为该点的xyz坐标*/
            glm::vec4 now(modelBunny * glm::vec4(vertices[selectedPointIndice * 6], vertices[selectedPointIndice * 6 + 1], vertices[selectedPointIndice * 6 + 2], 1.0f));
        	
            /*创建一个平移矩阵
             *第一个参数为目标矩阵，第二个矩阵是平移的方向向量*/
			model = glm::translate(glm::mat4(1.0f), glm::vec3(now.x, now.y, now.z));

            /*model是一个小立方体
             *scale用于创建一个缩放矩阵*/
            model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));	
            view = camera.GetViewMatrix();	
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);	
           
			//着色器设置相关矩阵参数
			selectShader.setMat4("projection", projection); 
            selectShader.setMat4("view", view);
            selectShader.setMat4("model", model);
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);	
        }
    	
    	/*glfwSwapBuffers函数会交换颜色缓冲
    	 *它是一个储存着GLFW窗口每一个像素颜色的大缓冲），
    	 *它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上。*/
        glfwSwapBuffers(window);
    	/*glfwPollEvents函数检查有没有触发什么事件（比如键盘输入、鼠标移动等），
    	 *然后调用对应的回调函数（可以通过回调方法手动设置）。
    	 *我们一般在循环的开始调用事件处理函数。*/
        glfwPollEvents();
    }

    // 实现目标后，取消所有资源分配
    glDeleteVertexArrays(1, &bunnyVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &bunnyVBO);
    glDeleteBuffers(1, &lightVBO);

    // 最后调用glfwTerminate函数来释放GLFW分配的资源
    glfwTerminate();
    return 0;
}

//输入函数
void do_movement(){
    GLfloat bunnySpeed = 70.0f * deltaTime;
    // 照相机向z轴负方向移动
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    // 照相机向z轴正方向移动
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    // 照相机向x轴负方向
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    // 照相机向x轴正方向
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    // <- 兔子向左移动
    if (keys[GLFW_KEY_LEFT])
        modelBunny = glm::translate(modelBunny, glm::vec3(-bunnySpeed, 0, 0));
    // -> 兔子向右移动
    if (keys[GLFW_KEY_RIGHT])
        modelBunny = glm::translate(modelBunny, glm::vec3(bunnySpeed, 0, 0));
    // J 兔子绕y轴顺时针转
    if (keys[GLFW_KEY_J])
        modelBunny = glm::rotate(modelBunny, glm::radians(bunnySpeed), glm::vec3(0.f, 0.f, 1.f));
    // L 兔子绕y轴逆时针转
    if (keys[GLFW_KEY_L])
        modelBunny = glm::rotate(modelBunny, glm::radians(-bunnySpeed), glm::vec3(0.f, 0.f, 1.f));
    // I 兔子向前转（绕x轴顺时针）
    if (keys[GLFW_KEY_I])
        modelBunny = glm::rotate(modelBunny, glm::radians(-bunnySpeed), glm::vec3(1.f, 0.f, 0.f));
    // K 兔子向后转（绕x轴逆时针）
    if (keys[GLFW_KEY_K])
        modelBunny = glm::rotate(modelBunny, glm::radians(bunnySpeed), glm::vec3(1.f, 0.f, 0.f));
    // Z 兔子缩小
    if (keys[GLFW_KEY_Z])
        modelBunny = glm::scale(modelBunny, glm::vec3(1.0f - 0.01f * bunnySpeed, 1.0f - 0.01f * bunnySpeed, 1.0f - 0.01f * bunnySpeed));
    // X 兔子放大
    if (keys[GLFW_KEY_X])
        modelBunny = glm::scale(modelBunny, glm::vec3(1.0f + 0.01f * bunnySpeed, 1.0f + 0.01f * bunnySpeed, 1.0f + 0.01f * bunnySpeed));
}

//处理所有输入：查询GLFW是否在此帧中按下/释放了相关的按键并做出相应的反应
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode){
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
    { //如果按下“=”键
        isAttenuation = !isAttenuation;
        std::cout << "点光源衰减： ";
        if(isAttenuation) std::cout << "打开.\n";
        else std::cout << "关闭.\n";
        std::cout << SPLIT << std::endl;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        isFlashlight = !isFlashlight;
        std::cout << "FlashLight（手电筒）： ";
        if(isFlashlight) std::cout << "打开.\n";
        else std::cout << "关闭.\n";
        std::cout << SPLIT << std::endl;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    { //如果按下esc则关闭窗口
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {	//获取当前的输入模式，判断光标是否禁用
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // 如果禁用，则恢复使用光标
            cursorDisabled = false;	//禁用标志
        }else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//如果当前光标不被禁用，则禁用光标（离开拾取模式）
            cursorDisabled = true;
            firstMouse = true;
        }
    }
    if (key >= 0 && key < 1024)
    {	//判断所有按键的按下与松开状态
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

// 每当改变窗口大小时，就会执行回调函数 
/* 当用户改变窗口的大小的时候，视口也应该被调整。
   对窗口注册一个回调函数(Callback Function)，它会在每次窗口大小被调整的时候被调用*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//任何时候移动鼠标调用该回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    mouseX = xpos;	//鼠标x坐标
    mouseY = ypos;	//鼠标y坐标
    if(cursorDisabled)   //如果光标禁用
    {	
        if (firstMouse)        //如果为第一次进入 记录当前鼠标位置为初始化鼠标位置
        {
            lastX = xpos;	
            lastY = ypos;
            firstMouse = false;	
        }

		//计算x y坐标的偏移量（当前帧和上一帧的位置差）
        float xoffset = xpos - lastX;	
        float yoffset = lastY - ypos; 
    	/*注意：
    	 *这里如果不反过来 实际操作过程中向上移动摄像机 物体也会跟着向上移动 实际应该向下*/
        lastX = xpos;
        lastY = ypos;
        camera.ProcessMouseMovement(xoffset, yoffset);	//相机根据坐标偏移量移动
    }
}

//任何时候鼠标滚轮滚动时都会调用该函数。（响应鼠标滚轮）
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset); //yoffset值代表滚动的大小
}

// 响应鼠标点击事件
void mouse_click_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!cursorDisabled && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) //当光标未被禁用且鼠标左键按下的时候
    {
		//鼠标的xy坐标
		GLfloat xpos = mouseX;	
        GLfloat ypos = mouseY;
        std::cout << "选取点的屏幕坐标（屏幕左上角为0,0） : " << xpos << ' ' << ypos << std::endl;

        GLfloat minDistance = glm::pow(10, 10);	
        GLuint  minIndex = 0;	
        GLfloat minX =  0.0f, minY = 0.0f;
        for (int i = 0; i < vertices_size; i++)
        {
			/*	当模型发生不均匀缩放会破坏法向量导致光线扭曲。
				解决方法：使用正规矩阵（模型矩阵左上角的逆矩阵的转置矩阵）
	            modelBunny三阶子矩阵 * 点的法线  点乘 相机的方向 <0 及这个点在正面上
				判断哪些点应该被拾取 */
			if (glm::dot(glm::mat3(glm::transpose(glm::inverse(modelBunny))) * glm::vec3(vertices[6 * i + 3], vertices[6 * i + 4], vertices[6 * i + 5]),
                        camera.Front) < 0)
			{
				glm::vec4 iPos;
				glm::mat4 view = camera.GetViewMatrix();	
				glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);	

				/*WorldSpace：M.model * local
				 * model矩阵*顶点坐标
				 */
				iPos = modelBunny * glm::vec4(vertices[i * 6 + 0], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
				/*V.clip = M.projection * M.view * M.model * M.local
				 *在WorldSpace的基础上乘view视野矩阵转换到照相机坐标系/视野坐标系
				 * Projection投影矩阵把对象变化到四维裁剪坐标系
				 */
				iPos = projection * view * iPos;
				/*将 Clip Space（四维裁剪坐标系）经过透视除法转换为三维规范化设备坐标系
				 *视口变换再转换为 Screen Space（窗口坐标系）上的二维坐标
				 *左上角为(0,0) */
				GLfloat pointPosX = SCR_WIDTH / 2 * (iPos.x / iPos.w) + SCR_WIDTH / 2;  	//点的x坐标
				GLfloat pointPosY = SCR_HEIGHT / 2 * (-iPos.y / iPos.w) + SCR_HEIGHT / 2;	//点的y坐标

				/*判断当前点与所有应该拾取的点之间的距离，并不断的更新minDistance直到所有的点被计算完，
				  得出光标点击的点与离它最近的点之前的距离记录为minDistance*/
				if ((pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos) < minDistance)
 {
					minDistance = (pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos);
					minIndex = i;
					minX = pointPosX;
					minY = pointPosY;
				}
			}
		}
        // 如果minDistance小于20个像素
        if (minDistance < 400){
            selectedPointIndice = minIndex;
            std::cout << "点的索引号 : " << minIndex << std::endl;
            std::cout << "点的坐标 : " << vertices[minIndex * 6 + 0] << ' ' << vertices[minIndex * 6 + 1] << ' ' << vertices[minIndex * 6 + 2] << std::endl;
            std::cout << "点的屏幕坐标 : " << minX << ' ' << minY << std::endl;
        }else{
            std::cout << "附近没有点(光标与最近点之间的距离必须小于20个像素)" << std::endl;
        }
        std::cout << SPLIT << std::endl;
    }
}