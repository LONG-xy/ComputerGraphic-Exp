#version 330 core
out vec4 FragColor;

struct Material 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct PointLight 
{
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight 
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform PointLight pointLights;
uniform SpotLight spotLight;
uniform Material material;

// function prototypes
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light , vec3 normal , vec3 fragPos, vec3 viewDir);

void main(){    
    // 一些属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // == =====================================================
    // Our lighting is set up in 2 phases: a point light and a flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: point lights
    vec3 result = CalcPointLight(pointLights, norm, FragPos, viewDir);    
    // phase 2: spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);
	
    // diffuse shading 计算漫反射强度
    float diff = max(dot(normal, lightDir), 0.0);
	
    // specular shading 计算镜面反射强度
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
    // attenuation 计算衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
	
    // combine results 合并各个光照分量
    vec3 ambient = light.ambient * material.ambient; //环境光
    vec3 diffuse = light.diffuse * diff * material.diffuse;//漫反射
    vec3 specular = light.specular * spec * material.specular;//镜面反射
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);            //点光源颜色
}

// 计算使用聚光灯的颜色
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	//光线向量通过光源位置向量与片段位置向量相减
    vec3 lightDir = normalize(light.position - fragPos); 
    // diffuse shading
	//计算漫反射强度
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
	//viewDir为观察者向量。
	//reflectDir是反射光线向量，reflect是求反射向量的函数其中第一个参数是入射光线，它必须是从光源出发，所以lightDir要取反。
    vec3 reflectDir = reflect(-lightDir, normal);
	//计算镜面反射强度
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //计算衰减系数
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity 平滑/软化边缘
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // 计算各个光照分量
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff* material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
	//各个光照分量乘上衰减系数
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);    //聚光灯颜色
}