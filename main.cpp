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

//��ʼ�������λ��
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

//�Ƿ�Ϊ��һ�ν������
bool  firstMouse = true;

// ʱ�������������Ⱦ��֮֡�� ��ʱ�䣩
float deltaTime = 0.0f;  //��ǰ֡����һ֡��ʱ���
float lastFrame = 0.0f;  //��һ֡��ʱ��

// ���Ӳ���
glm::mat4 modelBunny = glm::mat4(1.0f);// �Ľ׵�λ��
GLuint vertices_size, indices_size;	     // ��������Լ���Ƭ����

// ���ղ���(����һ��ȫ��vec3��������ʾ��Դ�ڳ���������ռ������е�λ��)
glm::vec3 PointlightPos(5.0f, 13.0f, 4.0f);	// ��ά���� ����

//����������ʼ������λ�� ��Ļ���ģ�
GLfloat mouseX = SCR_WIDTH / 2.0;
GLfloat mouseY = SCR_HEIGHT / 2.0;

GLuint selectedPointIndice = 0; 

const int MAXPOINT = 40000; //.py�ļ� 34834
const int MAXINDEX = 70000; //.py�ļ� 69451

const char* DataFile = "bunny_normal.ply2";
const char *SPLIT = "--------------------------------------------------------------";
GLfloat vertices[MAXPOINT*6]; //����ÿ��Ҫ��6�����ݣ�x,y,z ��������
GLuint indices[MAXINDEX*3];   //��Ƭÿ����3�����ݣ�����������ţ�

bool keys[1024];	//��ȡ����
bool isAttenuation = false;
bool isFlashlight = false;
bool cursorDisabled = true; //���������ѡ����

//����˵��
void KeyDescription()
{
    std::cout << "=: ���Դ˥��\n";
    std::cout << "P: FlashLight����\n";
    std::cout << "A: �������.\n";
    std::cout << "D: �������.\n";
    std::cout << "W: �����ǰ\n";
    std::cout << "S: ������.\n";
    std::cout << "�����<-: ��������.\n";
    std::cout << "�����->: ��������.\n";
    std::cout << "J: ����˳ʱ����ת.\n";
    std::cout << "L: ������ʱ����ת.\n";
    std::cout << "I: ������ǰ��ת.\n";
    std::cout << "K: ���������ת.\n";
    std::cout << "X: ���ӷŴ�.\n";
    std::cout << "Z: ������С.\n";
    std::cout << "Tab: �߿�ģʽ�����ù�꣩| Ĭ��ģʽ�����ù�꣩\n";
    std::cout << SPLIT << std::endl;
}

//���ڳ�ʼ������
GLFWwindow* init(){
    //��ʼ��glfw��
    glfwInit();
	
    //�����汾�����
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//���汾
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�ΰ汾
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //����GLFW���ڲ��������С�����ƣ������Ƿ񴴽��ɹ�
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LONG_xy_Stanford bunny", NULL, NULL);
	
	//������ڴ���ʧ�ܣ�����ʧ����Ϣ
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();	//�ͷ�������Դ
        exit(EXIT_SUCCESS);
    }
    //���õ�ǰ�Ĵ���������
	/*�������֮����Ҫ�õ�ǰ���ڵĻ����ڵ�ǰ�߳��ϳ�Ϊ��ǰ������
	 *���ǽ������Ļ��ƶ��ử�ڸոմ����Ĵ�����*/
    glfwMakeContextCurrent(window);

	//�����ڵ�����С��ʱ������������
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//ע�ᰴ���ص�����
    glfwSetKeyCallback(window, key_callback);	

	//ע����ص���������������ƶ��¼� �ص������Ӧ����
    glfwSetCursorPosCallback(window, mouse_callback);	

    //ע����갴���ص�����
    glfwSetMouseButtonCallback(window, mouse_click_callback);
	
    //ע����ֻص�����
    glfwSetScrollCallback(window, scroll_callback);		

    // ��GLFW������꣨�������Ĺ�� ����һֱͣ���ڴ����У�
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: ��������openGL����ָ�루gladѰ��opengl�ĺ�����ַ������opengl�ĺ���ǰ��Ҫ��ʼ��glad��
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_SUCCESS);
    }
    // ����opengl��ȫ��״̬
    /*������Ȳ��ԣ�Ĭ���ǹرյ�
     *GLFW���Զ�����Z-buffer ��Ȼ���*/
    glEnable(GL_DEPTH_TEST);
    return window;
}

int main()
{
    KeyDescription();
    // glfw: ��ʼ������������ز���
	//���ô��ڳ�ʼ������
    GLFWwindow* window = init();

    //��ȡ��ɫ��·��
    Shader lightingShader("bunny.vs", "bunny.fs");	// vertexShader fragmentShader
    Shader lampShader("lamp.vs", "lamp.fs");		//����ɫ��
    Shader selectShader("select.vs", "select.fs");	//ʰȡ��ɫ��

    // ���ö������ݣ��ͻ������������ö�������
    //����ply2�ļ�
    parseNormal(DataFile, vertices, indices, vertices_size, indices_size);

   
	/*�����嶥������
	 *6���� ÿ����2�������� һ��������3���� 6*3*2  36������*/
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

    //��һ�����������ӵ�VAO & VBO & EBO
    GLuint  bunnyVAO, bunnyVBO, bunnyEBO;
	
    glGenVertexArrays(1, &bunnyVAO);	//����VAO����
    glBindVertexArray(bunnyVAO);        //��VAO����
	
    glGenBuffers(1, &bunnyVBO);         	    //����VBO
    glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);    //��VBO
    glBufferData(GL_ARRAY_BUFFER, 6 * vertices_size * sizeof(GLfloat), vertices, GL_STATIC_DRAW);    //���û�����Ķ������鸴�Ƶ������й�OpenGLʹ��
	
    glGenBuffers(1, &bunnyEBO);	                        //����EBO�����������        
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyEBO);	//��EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*indices_size*sizeof(GLuint), indices, GL_STATIC_DRAW);	  //����EBO���ݣ��������洢��EBO����

    // λ�����ԣ����ö�������ָ�룩 ���Ͷ������ݷ�ʽ
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);	//layout=0,vec3,���������������ļ��
	glEnableVertexAttribArray(0);	//������������ ������layout = 0��λ�����ݣ�
	
    // ����������
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1); //����layout = 1 ��������

    glBindVertexArray(0); // ���VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // ���VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);// ���EBO��


    // ��Σ����õƹ��VAO & VBO
    GLuint lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);            	//�󶨵ƹ�VAO����

    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);	//�󶨵ƹ�VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);	//�ƹⶥ�����ݰ󶨵�������
	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);	//֪ͨOPENGL����������	
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);             // ȡ����VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // ȡ����VBO
    
	//���ӵĳ�ʼ״̬
    modelBunny = glm::rotate(modelBunny, glm::radians(145.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//��y����ʱ����ת145��
   
	//while��Ⱦѭ���������ڹر���֮ǰ���ϻ���ͼ���ܹ������û����룩
    while (!glfwWindowShouldClose(window)) //��鴰���Ƿ���Ҫ�ر�
    {	
        //����������֡����Ⱦʱ���
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // ����
        do_movement();

    if (!cursorDisabled)//���δ����
    {
        glEnable(GL_CULL_FACE);                       //OpenGL��GL_CULL_FACEѡ��: �������޳�����
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	//��ʾ�߶Σ��������������ʾ
    }
    else  //��걻���� ��ʾ����������,����β������ʽ
      {
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    	
    // ��Ⱦ����
    	/*��ÿ���µ���Ⱦ������ʼ��ʱ����������ϣ���������������ܿ�����һ�ε�������Ⱦ�����
    	 *���ǿ���ͨ������glClear�����������Ļ����ɫ���壬
    	 *������һ������λ(Buffer Bit)��ָ��Ҫ��յĻ��壬
    	 */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//�����ɫ�����Լ���Ȼ���

        // ���ø�ʽ/��ͼ����ǰ������ɫ��
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // �ƹ�����
        glm::vec3 lightColor;	
		
		//������ɫ
        lightColor.x = 3.0f;
        lightColor.y = 0.0f;
        lightColor.z = 7.0f;
    	
        glm::vec3 diffuseColor = lightColor   * glm::vec3(0.3f); 
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); 

        // ���Դ
        lightingShader.setVec3("pointLights.position", PointlightPos);
        lightingShader.setVec3("pointLights.ambient", ambientColor);
        lightingShader.setVec3("pointLights.diffuse", diffuseColor);
        lightingShader.setVec3("pointLights.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights.constant", 1.0f);
        lightingShader.setFloat("pointLights.linear", (isAttenuation ? 0.015 : 0.0f));
        lightingShader.setFloat("pointLights.quadratic", (isAttenuation ? 0.0007 : 0.0f));
        
        // �۹��
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

        // ��������
        lightingShader.setVec3("material.ambient", 0.5f, 0.5f, 0.5f);	
        lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.2f);	
        lightingShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);  
        lightingShader.setFloat("material.shininess", 250.0f);

    	//View & Projection transformation
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    	//Ϊ��ɫ���еľ���ֵ
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", modelBunny); //����ģ�Ͳ������뵽model��ɫ��

        // ��Ⱦ����
        glBindVertexArray(bunnyVAO);	//�����ӵ�VAO
        glDrawElements(GL_TRIANGLES, 3*indices_size, GL_UNSIGNED_INT, 0);//ͼԪ���ƺ��� 
        glBindVertexArray(0);	        //ȡ����VAO

        // �����ƵĶ���
        lampShader.use();

        // ���õƵ���ؾ���
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, PointlightPos);       //�ѵ�ƽ�Ƶ����Դ��λ�ã��������ţ���������ô����
        model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube���������Դ��
        lampShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

        //ʰȡ�������
        if (selectedPointIndice <= vertices_size)  //����������С�ڵ�ĸ���
        {	
            selectShader.use();	//������ɫ��
			/*����һ����ά������¼��ǰ���λ��
			 n1 n2 n3 �ֱ�Ϊ�õ��xyz����*/
            glm::vec4 now(modelBunny * glm::vec4(vertices[selectedPointIndice * 6], vertices[selectedPointIndice * 6 + 1], vertices[selectedPointIndice * 6 + 2], 1.0f));
        	
            /*����һ��ƽ�ƾ���
             *��һ������ΪĿ����󣬵ڶ���������ƽ�Ƶķ�������*/
			model = glm::translate(glm::mat4(1.0f), glm::vec3(now.x, now.y, now.z));

            /*model��һ��С������
             *scale���ڴ���һ�����ž���*/
            model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));	
            view = camera.GetViewMatrix();	
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);	
           
			//��ɫ��������ؾ������
			selectShader.setMat4("projection", projection); 
            selectShader.setMat4("view", view);
            selectShader.setMat4("model", model);
            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);	
        }
    	
    	/*glfwSwapBuffers�����ύ����ɫ����
    	 *����һ��������GLFW����ÿһ��������ɫ�Ĵ󻺳壩��
    	 *������һ�����б��������ƣ����ҽ�����Ϊ�����ʾ����Ļ�ϡ�*/
        glfwSwapBuffers(window);
    	/*glfwPollEvents���������û�д���ʲô�¼�������������롢����ƶ��ȣ���
    	 *Ȼ����ö�Ӧ�Ļص�����������ͨ���ص������ֶ����ã���
    	 *����һ����ѭ���Ŀ�ʼ�����¼���������*/
        glfwPollEvents();
    }

    // ʵ��Ŀ���ȡ��������Դ����
    glDeleteVertexArrays(1, &bunnyVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &bunnyVBO);
    glDeleteBuffers(1, &lightVBO);

    // ������glfwTerminate�������ͷ�GLFW�������Դ
    glfwTerminate();
    return 0;
}

//���뺯��
void do_movement(){
    GLfloat bunnySpeed = 70.0f * deltaTime;
    // �������z�Ḻ�����ƶ�
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    // �������z���������ƶ�
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    // �������x�Ḻ����
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    // �������x��������
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    // <- ���������ƶ�
    if (keys[GLFW_KEY_LEFT])
        modelBunny = glm::translate(modelBunny, glm::vec3(-bunnySpeed, 0, 0));
    // -> ���������ƶ�
    if (keys[GLFW_KEY_RIGHT])
        modelBunny = glm::translate(modelBunny, glm::vec3(bunnySpeed, 0, 0));
    // J ������y��˳ʱ��ת
    if (keys[GLFW_KEY_J])
        modelBunny = glm::rotate(modelBunny, glm::radians(bunnySpeed), glm::vec3(0.f, 0.f, 1.f));
    // L ������y����ʱ��ת
    if (keys[GLFW_KEY_L])
        modelBunny = glm::rotate(modelBunny, glm::radians(-bunnySpeed), glm::vec3(0.f, 0.f, 1.f));
    // I ������ǰת����x��˳ʱ�룩
    if (keys[GLFW_KEY_I])
        modelBunny = glm::rotate(modelBunny, glm::radians(-bunnySpeed), glm::vec3(1.f, 0.f, 0.f));
    // K �������ת����x����ʱ�룩
    if (keys[GLFW_KEY_K])
        modelBunny = glm::rotate(modelBunny, glm::radians(bunnySpeed), glm::vec3(1.f, 0.f, 0.f));
    // Z ������С
    if (keys[GLFW_KEY_Z])
        modelBunny = glm::scale(modelBunny, glm::vec3(1.0f - 0.01f * bunnySpeed, 1.0f - 0.01f * bunnySpeed, 1.0f - 0.01f * bunnySpeed));
    // X ���ӷŴ�
    if (keys[GLFW_KEY_X])
        modelBunny = glm::scale(modelBunny, glm::vec3(1.0f + 0.01f * bunnySpeed, 1.0f + 0.01f * bunnySpeed, 1.0f + 0.01f * bunnySpeed));
}

//�����������룺��ѯGLFW�Ƿ��ڴ�֡�а���/�ͷ�����صİ�����������Ӧ�ķ�Ӧ
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode){
    if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
    { //������¡�=����
        isAttenuation = !isAttenuation;
        std::cout << "���Դ˥���� ";
        if(isAttenuation) std::cout << "��.\n";
        else std::cout << "�ر�.\n";
        std::cout << SPLIT << std::endl;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        isFlashlight = !isFlashlight;
        std::cout << "FlashLight���ֵ�Ͳ���� ";
        if(isFlashlight) std::cout << "��.\n";
        else std::cout << "�ر�.\n";
        std::cout << SPLIT << std::endl;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    { //�������esc��رմ���
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {	//��ȡ��ǰ������ģʽ���жϹ���Ƿ����
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // ������ã���ָ�ʹ�ù��
            cursorDisabled = false;	//���ñ�־
        }else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//�����ǰ��겻�����ã�����ù�꣨�뿪ʰȡģʽ��
            cursorDisabled = true;
            firstMouse = true;
        }
    }
    if (key >= 0 && key < 1024)
    {	//�ж����а����İ������ɿ�״̬
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

// ÿ���ı䴰�ڴ�Сʱ���ͻ�ִ�лص����� 
/* ���û��ı䴰�ڵĴ�С��ʱ���ӿ�ҲӦ�ñ�������
   �Դ���ע��һ���ص�����(Callback Function)��������ÿ�δ��ڴ�С��������ʱ�򱻵���*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//�κ�ʱ���ƶ������øûص�����
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    mouseX = xpos;	//���x����
    mouseY = ypos;	//���y����
    if(cursorDisabled)   //���������
    {	
        if (firstMouse)        //���Ϊ��һ�ν��� ��¼��ǰ���λ��Ϊ��ʼ�����λ��
        {
            lastX = xpos;	
            lastY = ypos;
            firstMouse = false;	
        }

		//����x y�����ƫ��������ǰ֡����һ֡��λ�ò
        float xoffset = xpos - lastX;	
        float yoffset = lastY - ypos; 
    	/*ע�⣺
    	 *��������������� ʵ�ʲ��������������ƶ������ ����Ҳ����������ƶ� ʵ��Ӧ������*/
        lastX = xpos;
        lastY = ypos;
        camera.ProcessMouseMovement(xoffset, yoffset);	//�����������ƫ�����ƶ�
    }
}

//�κ�ʱ�������ֹ���ʱ������øú���������Ӧ�����֣�
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset); //yoffsetֵ��������Ĵ�С
}

// ��Ӧ������¼�
void mouse_click_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (!cursorDisabled && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) //�����δ�����������������µ�ʱ��
    {
		//����xy����
		GLfloat xpos = mouseX;	
        GLfloat ypos = mouseY;
        std::cout << "ѡȡ�����Ļ���꣨��Ļ���Ͻ�Ϊ0,0�� : " << xpos << ' ' << ypos << std::endl;

        GLfloat minDistance = glm::pow(10, 10);	
        GLuint  minIndex = 0;	
        GLfloat minX =  0.0f, minY = 0.0f;
        for (int i = 0; i < vertices_size; i++)
        {
			/*	��ģ�ͷ������������Ż��ƻ����������¹���Ť����
				���������ʹ���������ģ�;������Ͻǵ�������ת�þ���
	            modelBunny�����Ӿ��� * ��ķ���  ��� ����ķ��� <0 ���������������
				�ж���Щ��Ӧ�ñ�ʰȡ */
			if (glm::dot(glm::mat3(glm::transpose(glm::inverse(modelBunny))) * glm::vec3(vertices[6 * i + 3], vertices[6 * i + 4], vertices[6 * i + 5]),
                        camera.Front) < 0)
			{
				glm::vec4 iPos;
				glm::mat4 view = camera.GetViewMatrix();	
				glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);	

				/*WorldSpace��M.model * local
				 * model����*��������
				 */
				iPos = modelBunny * glm::vec4(vertices[i * 6 + 0], vertices[i * 6 + 1], vertices[i * 6 + 2], 1.0f);
				/*V.clip = M.projection * M.view * M.model * M.local
				 *��WorldSpace�Ļ����ϳ�view��Ұ����ת�������������ϵ/��Ұ����ϵ
				 * ProjectionͶӰ����Ѷ���仯����ά�ü�����ϵ
				 */
				iPos = projection * view * iPos;
				/*�� Clip Space����ά�ü�����ϵ������͸�ӳ���ת��Ϊ��ά�淶���豸����ϵ
				 *�ӿڱ任��ת��Ϊ Screen Space����������ϵ���ϵĶ�ά����
				 *���Ͻ�Ϊ(0,0) */
				GLfloat pointPosX = SCR_WIDTH / 2 * (iPos.x / iPos.w) + SCR_WIDTH / 2;  	//���x����
				GLfloat pointPosY = SCR_HEIGHT / 2 * (-iPos.y / iPos.w) + SCR_HEIGHT / 2;	//���y����

				/*�жϵ�ǰ��������Ӧ��ʰȡ�ĵ�֮��ľ��룬�����ϵĸ���minDistanceֱ�����еĵ㱻�����꣬
				  �ó�������ĵ�����������ĵ�֮ǰ�ľ����¼ΪminDistance*/
				if ((pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos) < minDistance)
 {
					minDistance = (pointPosX - xpos) * (pointPosX - xpos) + (pointPosY - ypos) * (pointPosY - ypos);
					minIndex = i;
					minX = pointPosX;
					minY = pointPosY;
				}
			}
		}
        // ���minDistanceС��20������
        if (minDistance < 400){
            selectedPointIndice = minIndex;
            std::cout << "��������� : " << minIndex << std::endl;
            std::cout << "������� : " << vertices[minIndex * 6 + 0] << ' ' << vertices[minIndex * 6 + 1] << ' ' << vertices[minIndex * 6 + 2] << std::endl;
            std::cout << "�����Ļ���� : " << minX << ' ' << minY << std::endl;
        }else{
            std::cout << "����û�е�(����������֮��ľ������С��20������)" << std::endl;
        }
        std::cout << SPLIT << std::endl;
    }
}