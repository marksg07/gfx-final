#include "RayScene.h"
#include "Settings.h"
#include "CS123SceneData.h"
#include "intersect/implicitshape.h"
#include "camera/Camera.h"
#include <iostream>
#include <thread>
#include "intersect/kdtree.h"
#include <sys/time.h>
#include <functional>


inline double get_time(void) {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_usec * 0.000001 + t.tv_sec);
}

RayScene::RayScene(Scene &scene) :
    Scene(scene),
    m_camTransform(),
    m_invTransform(),
    m_eye()
{
    m_nodes = std::vector<object_node_t>(scene.m_nodes);

    m_lights = std::vector<CS123SceneLightData>(scene.m_lights);
    m_global = scene.m_global;
    m_textures = std::map<std::string, std::unique_ptr<QImage>>();
    m_textures.clear();
    printf("Textures loaded:\n");
    int ii = 1;
    for(std::map<std::string, std::unique_ptr<QImage>>::iterator it = scene.m_textures.begin(); it != scene.m_textures.end(); it++, ii++) {
        if(it->second == nullptr)
            continue;
        printf("%s maps to %p, width = %d\n", it->first.data(), it->second.get(), it->second->width());
        fflush(stdout);
        m_textures[it->first] = std::make_unique<QImage>(*(it->second.get())); //std::make_shared<QImage>(*(it->second));
    }
    //printf("Forceloading: image = %p, width = %d\n", m_textures["image/marsTexture.png"].get(), m_textures["image/marsTexture.png"]->width());
        // get bounds of scene first
    glm::vec3 minbound(INFINITY, INFINITY, INFINITY);
    glm::vec3 maxbound(-INFINITY, -INFINITY, -INFINITY);
    for(unsigned long i = 0; i < m_nodes.size(); i++) {
        minbound.x = glm::min(minbound.x, m_nodes[i].minbound.x);
        minbound.y = glm::min(minbound.y, m_nodes[i].minbound.y);
        minbound.z = glm::min(minbound.z, m_nodes[i].minbound.z);
        maxbound.x = glm::max(maxbound.x, m_nodes[i].maxbound.x);
        maxbound.y = glm::max(maxbound.y, m_nodes[i].maxbound.y);
        maxbound.z = glm::max(maxbound.z, m_nodes[i].maxbound.z);
    }
    if(settings.useKDTree) {
        printf("kd-tree enabled, building now\n");
        fflush(stdout);
        // NOTE: clock() doesn't work with multithreading b/c it counts clocks over all cores
        // so it looked like multithreading was running slower than normal non-multithreading
        //std::clock_t start = clock();
        double start = get_time();
        m_kdtree = KDTree::buildTree(m_nodes, 0, minbound, maxbound);
        printf("kd-tree finished building, took %f secs\n", get_time() - start);
        fflush(stdout);
    }
    //m_kdtree->pprint();
    //printf("my father has %d objs but i have %d objs\n", scene.m_nodes.size(), m_nodes.size());
    // TODO [INTERSECT]
    // Remember that any pointers or OpenGL objects (e.g. texture IDs) will
    // be deleted when the old scene is deleted (assuming you are managing
    // all your memory properly to prevent memory leaks).  As a result, you
    // may need to re-allocate some things here.
}

/*void RayScene::fill(CS123ISceneParser *parser) {
    Scene.parse(this, parser);
}*/

void RayScene::setDrawParams(Camera *camera, int width, int height) {
    m_camTransform = camera->getScaleMatrix() * camera->getViewMatrix();
    m_invTransform = glm::inverse(m_camTransform);
    m_eye = glm::inverse(camera->getViewMatrix()) * glm::vec4(0, 0, 0, 1);
    m_width = width;
    m_height = height;
}

struct rgbfloat {
    double r, g, b;
};

glm::vec3 gouraudIlluminate(glm::vec4 point, glm::vec4 normal, CS123SceneMaterial mat,
                       CS123SceneGlobalData global, std::vector<CS123SceneLightData> lights) {

    glm::vec4 rgba = mat.cAmbient;
    for(unsigned long i = 0; i < lights.size(); i++) {
        CS123SceneLightData *light = &lights[i];
        glm::vec4 pToL;
        if(light->type == LightType::LIGHT_POINT && settings.usePointLights)
            pToL = light->pos - point;
        else if(light->type == LightType::LIGHT_DIRECTIONAL && settings.useDirectionalLights)
            pToL = -light->dir;
        else
            continue;
        pToL = glm::normalize(pToL);
        float kddot = glm::dot(normal, pToL);
        kddot = glm::clamp(kddot, 0.f, 1.f);
        rgba += light->color * mat.cDiffuse * kddot;
    }
    return (255.f * glm::clamp(rgba, 0.f, 1.f)).xyz();
}

bool isSignificant(glm::vec4 v) {
    const double epsilon = 0.000001;
    return v.x > epsilon && v.y > epsilon && v.z > epsilon;
}

inline glm::vec4 getQImageColor(QImage *image, int s, int t) {
    QRgb col = image->pixel(s, t);
    return glm::vec4(qRed(col), qGreen(col), qBlue(col), 0) / 255.f;
}

glm::vec3 colorFromRaySingleObj(RayScene *scene, const object_node_t *obj, glm::vec4 P_ws, glm::vec4 d_ws, int recurseLevel, float recurseWeight);

glm::vec4 sampleBilinear(QImage *image, float u, float v, float repu, float repv) {
    int w = image->width(), h = image->height();
    float fs = u * repu * w;
    float ft = v * repv * h;
    double is, it;
    fs = modf(fs, &is);
    ft = modf(ft, &it);
    // int parts
    int s = is + 0.1;
    int t = it + 0.1;
    int sn = s + 1;
    int tn = t + 1;
    s = (s % w + w) % w;
    sn = (sn % w + w) % w;
    t = (t % h + h) % h;
    tn = (tn % h + h) % h;

    glm::vec4 tl = getQImageColor(image, s, t);
    glm::vec4 tr = getQImageColor(image, sn, t);
    glm::vec4 bl = getQImageColor(image, s, tn);
    glm::vec4 br = getQImageColor(image, sn, tn);
    glm::vec4 top = glm::mix(tl, tr, fs);
    glm::vec4 bot = glm::mix(bl, br, fs);
    glm::vec4 texCol = glm::mix(top, bot, ft);
    return texCol;
}

glm::vec4 sampleBicubic(QImage *image, float u, float v, float repu, float repv) {
    int w = image->width(), h = image->height();
    float fs = u * repu * w;
    float ft = v * repv * h;
    double is, it;
    fs = modf(fs, &is);
    ft = modf(ft, &it);
    // int parts
    int s = is + 0.1;
    int t = it + 0.1;
    int sn = s + 1;
    int tn = t + 1;
    s = (s % w + w) % w;
    sn = (sn % w + w) % w;
    t = (t % h + h) % h;
    tn = (tn % h + h) % h;

    glm::vec4 tl = getQImageColor(image, s, t);
    glm::vec4 tr = getQImageColor(image, sn, t);
    glm::vec4 bl = getQImageColor(image, s, tn);
    glm::vec4 br = getQImageColor(image, sn, tn);
    glm::vec4 top = glm::mix(tl, tr, 3*fs*fs-2*fs*fs*fs);
    glm::vec4 bot = glm::mix(bl, br, 3*fs*fs-2*fs*fs*fs);
    glm::vec4 texCol = glm::mix(top, bot, 3*ft*ft-2*ft*ft*ft);
    return texCol;
}

const glm::mat4x4 bicubicMult(1, 0, -3, -2, 0, 0, 3, -2, 0, 1, -2, 1, 0, 0, -1, 1);
const glm::mat4x4 bicubicMultT = glm::transpose(bicubicMult);

inline glm::mat4x4 buildBCMatrix(float tl, float tr, float bl, float br) {
    return glm::mat4x4(tl, tr, tl, tr, bl, br, bl, br, tl, tr, tl, tr, bl, br, bl, br);
}

float interpCubic(float a, float b, float c, float d, float t) {
    return b + 0.5 * t * (-a + c + t * (2*a - 5*b + 4*c - d +
                                       t * (-a + 3*b - 3*c + d)));
}

// this is a "better" bicubic interpolation that uses 16 points instead of just 4. It looked a little better,
// but was much slower.
float sampleRealBicubicGray(QImage *image, float u, float v, float repu, float repv) {
    int w = image->width(), h = image->height();
    float fs = u * repu * w;
    float ft = v * repv * h;
    double is, it;
    fs = modf(fs, &is);
    ft = modf(ft, &it);
    // int parts of topleft coords
    int s = is - 0.9;
    int t = it - 0.9;
    float interps[4];
    int as = (s % w + w) % w, bs = ((s+1) % w + w) % w, cs = ((s+2) % w + w) % w, ds = ((s+3) % w + w) % w;
    for(int y = 0; y < 4; y++) {
        int yt = ((t+y) % h + h) % h;
        // for each row, get the 4 colors and interp
        float a = qRed(image->pixel(as, yt)) / 255.f;
        float b = qRed(image->pixel(bs, yt)) / 255.f;
        float c = qRed(image->pixel(cs, yt)) / 255.f;
        float d = qRed(image->pixel(ds, yt)) / 255.f;
        interps[y] = interpCubic(a, b, c, d, fs);
    }
    return interpCubic(interps[0], interps[1], interps[2], interps[3], ft);
}

glm::vec3 fullIlluminate(glm::vec4 point, glm::vec4 normal, glm::vec3 tangent, glm::vec3 bitangent, glm::vec2 texcor, glm::vec4 eye, CS123SceneMaterial mat,
                         CS123SceneGlobalData global, std::vector<CS123SceneLightData> lights, RayScene *scene, int recurseLevel,
                         float recurseWeight, const object_node_t *obj) {

    glm::vec4 rgba = mat.cAmbient;
    glm::vec4 pToEye = glm::normalize(eye - point);
    if(std::isnan(pToEye.x))
        pToEye = glm::vec4(1, 0, 0, 0);
    float epsilon = 0.0005;
    glm::vec4 diffuse_blended = mat.cDiffuse;
    bool texMap = settings.useTextureMapping && mat.textureMap.isUsed && texcor.x >= 0.f && texcor.x <= 1.f && texcor.y >= 0.f && texcor.y <= 1.f;
    bool bumpMap = settings.useBumpMapping && mat.bumpMap.isUsed && texcor.x >= 0.f && texcor.x <= 1.f && texcor.y >= 0.f && texcor.y <= 1.f;
    if(bumpMap) {
        assert(scene->m_textures.count(mat.bumpMap.filename) > 0);
        auto image = scene->m_textures[mat.bumpMap.filename].get();
        float xInc = 1./mat.bumpMap.repeatU/image->width();
        float yInc = 1./mat.bumpMap.repeatV/image->height();
        int depth = settings.bumpDepth;
        glm::vec3 norm3(normal);
        glm::mat3x3 TBN(tangent, -bitangent, norm3); // tangent space to world space
        glm::mat3x3 TBNI(glm::transpose(TBN)); // world space to tangent space
        if(settings.useSteepParallax) {
            glm::vec3 tanToEye(TBNI * glm::vec3(pToEye));
            glm::vec3 tanMaxPoint(tanToEye * (depth / tanToEye.z));
            int nsteps = 60;
            float stepSize = 1./(nsteps);
            float u_start = 0;
            float v_start = 0;
            float uinc = -tanMaxPoint.x * xInc * stepSize;
            float vinc = -tanMaxPoint.y * yInc * stepSize;
            float height = 1;
            float samp = sampleBicubic(image, texcor.x, texcor.y, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r;
            while(samp < height) {
                height -= stepSize;
                u_start += uinc;
                v_start += vinc;
                samp = sampleBicubic(image, texcor.x + u_start, texcor.y + v_start, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r;
            }
            texcor.x += u_start;
            texcor.y += v_start;
        }
        else if(settings.useParallax) {
            float meTest = sampleBicubic(image, texcor.x, texcor.y, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r * depth - depth;
            glm::vec3 tanToEye(TBNI * glm::vec3(pToEye));
            glm::vec3 parallaxPoint(tanToEye * meTest);
            texcor.x += parallaxPoint.x * xInc;
            texcor.y += parallaxPoint.y * yInc;
        }
        float u = texcor.x, v = texcor.y;

        float me = sampleBicubic(image, u, v, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r * depth - depth;
        float rt = sampleBicubic(image, u + xInc, v, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r * depth - depth;
        float bot = sampleBicubic(image, u, v + yInc, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r * depth - depth;
        float lt = sampleBicubic(image, u - xInc, v, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r * depth - depth;
        float top = sampleBicubic(image, u, v - yInc, mat.bumpMap.repeatU, mat.bumpMap.repeatV).r * depth - depth;
        // In tangent space: I am at (0,0,me)
        // rt is at (1,0,rt)
        // bot is at (0,1,bot)
        // ...
        // To transform from tangent space to WS, we just have to do
        // T*x - BT*y + N*z (-BT because BT is in -v direction)

        glm::vec3 me_ws = me * norm3;
        glm::vec3 r_ws = rt * norm3 + tangent;
        glm::vec3 b_ws = bot * norm3 - bitangent;
        glm::vec3 l_ws = lt * norm3 - tangent;
        glm::vec3 t_ws = top * norm3 + bitangent;
        int n_tris = 4;
        float fac = 1./n_tris;

        normal = fac * glm::vec4(glm::normalize(glm::cross(me_ws - r_ws, b_ws - me_ws)), 0.f);
        normal += fac * glm::vec4(glm::normalize(glm::cross(me_ws - b_ws, l_ws - me_ws)), 0.f);
        normal += fac * glm::vec4(glm::normalize(glm::cross(me_ws - l_ws, t_ws - me_ws)), 0.f);
        normal += fac * glm::vec4(glm::normalize(glm::cross(me_ws - t_ws, r_ws - me_ws)), 0.f);
    }
    if(texMap) {
        assert(scene->m_textures.count(mat.textureMap.filename) > 0);
        auto image = scene->m_textures[mat.textureMap.filename].get();
        glm::vec4 texCol = sampleBicubic(image, texcor.x, texcor.y, mat.textureMap.repeatU, mat.textureMap.repeatV);
        diffuse_blended = mat.cDiffuse * (1 - mat.blend) + texCol * mat.blend;
    }
    for(unsigned long i = 0; i < lights.size(); i++) {
        CS123SceneLightData *light = &lights[i];
        glm::vec4 pToL;
        float attenuation;
        float dist;
        if(light->type == LightType::LIGHT_POINT && settings.usePointLights) {
            pToL = light->pos - point;
            dist = glm::length(pToL);
            float inv_att = (light->function.x + light->function.y*dist + light->function.z*dist*dist);
            if(inv_att <= 1.f)
                attenuation = 1.f;
            else
                attenuation = 1.f / inv_att;
            pToL /= dist;
        }
        else if(light->type == LightType::LIGHT_DIRECTIONAL && settings.useDirectionalLights) {
            pToL = glm::normalize(-light->dir);
            dist = INFINITY;
            attenuation = 1.f;
        }
        else if(light->type == LightType::LIGHT_SPOT && settings.useSpotLights) {
            pToL = light->pos - point;
            dist = glm::length(pToL);
            pToL = glm::normalize(pToL);
            float cangle = glm::cos(light->angle);
            float cedge = glm::cos(light->angle + light->penumbra);
            float cvecs = glm::dot(-pToL, light->dir);
            if(cvecs <= cedge)
                continue;
            float inv_att = (light->function.x + light->function.y*dist + light->function.z*dist*dist);
            if(inv_att <= 1.f)
                attenuation = 1.f;
            else
                attenuation = 1.f / inv_att;
            if(cvecs < cangle) {
                // penumbration
                float a = 1 - glm::pow(((cvecs - cangle) / (cedge - cangle)), 5.f); // falloff of penumbra
                attenuation *= a;
            }
        }
        else
            continue;
        if(settings.useShadows) {
            float t_intersect = RayScene::rayIntersect(scene, point + epsilon * pToL, pToL);
            if(glm::length((t_intersect+epsilon) * pToL) < dist) // something obstructing path to light
                continue;
        }
        float kddot = glm::dot(normal, pToL);
        kddot = glm::clamp(kddot, 0.f, 1.f);
        glm::vec4 diffuse = diffuse_blended * kddot;
        glm::vec4 reflected = glm::reflect(-pToL, normal); // light to point ref. off norm

        float rvdot = glm::dot(reflected, pToEye);
        rvdot = glm::clamp(rvdot, 0.f, 1.f);
        glm::vec4 specular = mat.cSpecular * glm::pow(rvdot, mat.shininess);

        rgba += attenuation * light->color * (diffuse + specular);
    }
    // recurseWeight is intended to make it so that if you have a long recursion of reflections and refractions,
    // instead of having 2^(maxRecursion) bounces, recursion is stopped at minWeight.
    bool doReflect = settings.useReflection && recurseLevel < maxRecursion && recurseWeight >= minWeight && isSignificant(mat.cReflective);
    bool doRefract = settings.useRefraction && recurseLevel < maxRecursion && recurseWeight >= minWeight && isSignificant(mat.cTransparent);

    if(doReflect) {
        // if eye to point and normal are < 90 degrees apart, our eye is looking at a backface. This means we have refracted inside the object.
        bool insideObj = glm::dot(-pToEye, normal) >= 0;
        //printf("ref dot norm is %f, ref %f norm %f\n", glm::dot(reflectDir, normal), reflectDir.x, normal.x);
        // now recurse
        glm::vec4 colret;
        if(insideObj) { // the reflection will 100% hit the same object again.
            glm::vec4 reflectDir = glm::reflect(-pToEye, -normal); // reflect look off normal inside obj
            colret = mat.cReflective * glm::vec4(colorFromRaySingleObj(scene, obj, point + epsilon * reflectDir, reflectDir, recurseLevel + 1, recurseWeight * std::max(mat.cReflective.x, std::max(mat.cReflective.y, mat.cReflective.z))), 0.f);
        }
        else { // who knows. We could define a function that skips this obj, but whatever.
            glm::vec4 reflectDir = glm::reflect(-pToEye, normal); // reflect look off normal
            colret = mat.cReflective * glm::vec4(RayScene::colorFromRay(scene, point + epsilon * reflectDir, reflectDir, recurseLevel + 1, recurseWeight * std::max(mat.cReflective.x, std::max(mat.cReflective.y, mat.cReflective.z))), 0.f);
        }
        rgba += colret;
    }
    if(doRefract) {
        bool insideObj = glm::dot(-pToEye, normal) >= 0;
        glm::vec4 colRefract;
        if(insideObj) {
            // we use -normal because normal is pointing outwards
            glm::vec4 refractDir = glm::refract(-pToEye, -normal, mat.ior);
            if(refractDir == glm::vec4(0.f)) {
                colRefract = glm::vec4(0.f);
            }
            else {
                // we are inside the object, so we will refract outside. who knows what we hit.
                colRefract = mat.cTransparent * glm::vec4(RayScene::colorFromRay(scene, point + epsilon * refractDir, refractDir, recurseLevel + 1, recurseWeight * std::max(mat.cTransparent.x, std::max(mat.cTransparent.y, mat.cTransparent.z))), 0.f);
            }
        }
        else {
            glm::vec4 refractDir = glm::refract(-pToEye, normal, 1.f/mat.ior);
            assert(refractDir != glm::vec4(0.f));
            // we are outside the object, so we will refract inside, 100% hit same object again.
            colRefract = mat.cTransparent * glm::vec4(colorFromRaySingleObj(scene, obj, point + epsilon * refractDir, refractDir, recurseLevel + 1, recurseWeight * std::max(mat.cTransparent.x, std::max(mat.cTransparent.y, mat.cTransparent.z))), 0.f);
        }
        rgba += colRefract;
    }
    return glm::clamp(rgba, 0.f, 1.f).xyz();
}

glm::vec3 colorFromRaySingleObj(RayScene *scene, const object_node_t *obj, glm::vec4 P_ws, glm::vec4 d_ws, int recurseLevel, float recurseWeight) {
    glm::vec4 eye_os = obj->invtrans * P_ws;
    glm::vec4 v_dir_os = obj->invtrans * d_ws;
    auto t_p = ImplicitShape::getIntersectT(
                obj->primitive.type, eye_os, v_dir_os);
    if(t_p.pl == UNDEF || std::isinf(t_p.t) || std::isnan(t_p.t))
        return glm::vec3(0.f, 0.f, 0.f);
    glm::vec4 os_intersect = eye_os + (float)t_p.t * v_dir_os;
    glm::vec4 ws_intersect = P_ws + (float)t_p.t * d_ws;
    glm::vec4 os_N = glm::normalize(ImplicitShape::getNormal(t_p.pl, os_intersect));
    glm::vec4 ws_N = glm::normalize(glm::vec4((glm::transpose(obj->invtrans) * os_N).xyz(), 0.f));
    glm::vec2 texcor;
    glm::vec4 os_T;
    glm::vec3 ws_T, ws_BT;
    if(settings.useTextureMapping || settings.useBumpMapping) {
        texcor = ImplicitShape::getTexCoords(t_p.pl, os_intersect);
        os_T = glm::normalize(ImplicitShape::getTangent(t_p.pl, os_intersect));
        ws_T = glm::normalize((glm::transpose(obj->invtrans) * os_T).xyz());
        ws_BT = glm::normalize(glm::cross(glm::vec3(ws_N), ws_T));
    }
    glm::vec3 color = fullIlluminate(ws_intersect, ws_N, ws_T, ws_BT, texcor, P_ws, obj->primitive.material, scene->m_global, scene->m_lights, scene, recurseLevel, recurseWeight, obj);
    return color;
}

glm::vec3 RayScene::colorFromRay(RayScene *scene, glm::vec4 P_ws, glm::vec4 d_ws, int recurseLevel, float recurseWeight) {
    double smallestT = INFINITY;
    ISPlace isectPlace = UNDEF;
    const object_node_t *front_obj = NULL;
    glm::vec4 os_intersect;
    if(!settings.useKDTree) {
        //printf("itering thru obj, n_objs = %d\n", m_nodes.size());
        for(unsigned long i = 0; i < scene->m_nodes.size(); i++) {
            const object_node_t *obj = &scene->m_nodes[i];
            glm::vec4 eye_os = obj->invtrans * P_ws;
            glm::vec4 v_dir_os = obj->invtrans * d_ws;
            auto t_p = ImplicitShape::getIntersectT(obj->primitive.type, eye_os, v_dir_os);
            double t = t_p.t;
            if(t >= 0 && t < smallestT) {
                //printf("found new smallest intersection at %f\n", t);
                isectPlace = t_p.pl;
                smallestT = t;
                front_obj = obj;
                os_intersect = eye_os + glm::vec4(smallestT, smallestT, smallestT, 0) * v_dir_os;
            }
        }
    }
    else {
        assert(scene->m_kdtree != nullptr);
        struct ixInfo res = scene->m_kdtree->traverse(P_ws, d_ws);
        isectPlace = res.place;
        smallestT = res.t;
        front_obj = res.obj;
        os_intersect = res.ix;
    }
    if(isectPlace == UNDEF || std::isinf(smallestT) || std::isnan(smallestT))
        return glm::vec3(0.f, 0.f, 0.f);
    glm::vec4 ws_intersect = P_ws + glm::vec4(smallestT, smallestT, smallestT, 0) * d_ws;
    glm::vec4 os_N = glm::normalize(ImplicitShape::getNormal(isectPlace, os_intersect));
    glm::vec4 ws_N = glm::normalize(glm::vec4((glm::transpose(front_obj->invtrans) * os_N).xyz(), 0.f));
    glm::vec2 texcor;
    glm::vec4 os_T;
    glm::vec3 ws_T, ws_BT;
    if(settings.useTextureMapping || settings.useBumpMapping) {
        texcor = ImplicitShape::getTexCoords(isectPlace, os_intersect);
        os_T = glm::normalize(ImplicitShape::getTangent(isectPlace, os_intersect));
        ws_T = glm::normalize((glm::transpose(front_obj->invtrans) * os_T).xyz());
        ws_BT = glm::normalize(glm::cross(glm::vec3(ws_N), ws_T));
    }
    glm::vec3 color = fullIlluminate(ws_intersect, ws_N, ws_T, ws_BT, texcor, P_ws, front_obj->primitive.material, scene->m_global, scene->m_lights, scene, recurseLevel, recurseWeight, front_obj);
    return color;
}

double RayScene::rayIntersect(RayScene *scene, glm::vec4 P_ws, glm::vec4 d_ws) {
    double smallestT = INFINITY;
    if(!settings.useKDTree) {
        for(unsigned long i = 0; i < scene->m_nodes.size(); i++) {
            const object_node_t *obj = &scene->m_nodes[i];
            glm::vec4 eye_os = obj->invtrans * P_ws;
            glm::vec4 v_dir_os = obj->invtrans * d_ws;
            auto t_p = ImplicitShape::getIntersectT(obj->primitive.type, eye_os, v_dir_os);
            double t = t_p.t;
            if(t >= 0 && t < smallestT) {
                smallestT = t;
            }
        }
    }
    else {
        struct ixInfo res = scene->m_kdtree->traverse(P_ws, d_ws);
        if(res.place != UNDEF)
            smallestT = res.t;
    }
    return smallestT;
}

void RayScene::renderWithParams(RayScene *scene, BGRA *target, int ystart, int nrows, int nsamples, std::function<bool(int, int)> renderCondition) {
    int skipped = 0, notSkipped = 0;
    double samp_inc = 1./nsamples;
    double samp_off = samp_inc/2;
    double weight = samp_inc * samp_inc;
    for(int ypix = ystart; ypix < ystart + nrows; ypix++) {
        for(int xpix = 0; xpix < scene->m_width; xpix++) {
            if(renderCondition != nullptr && !renderCondition(xpix, ypix)) {
                skipped++;
                continue;
            }
            notSkipped++;
            double pr = 0, pg = 0, pb = 0;
            for(int ysamp = 0; ysamp < nsamples; ysamp++) {
                for(int xsamp = 0; xsamp < nsamples; xsamp++) {
                    double x = xpix + samp_off + xsamp * samp_inc;
                    double y = ypix + samp_off + ysamp * samp_inc;
                    // for each pixel look through m_nodes, color accoring to m_lights
                    glm::vec4 p_film(2. * x / scene->m_width - 1, 1 - 2. * y / scene->m_height, -1, 1);
                    glm::vec4 p_ws = scene->m_invTransform * p_film;
                    glm::vec4 v_dir = glm::normalize(p_ws - scene->m_eye);
                    glm::vec3 color = colorFromRay(scene, scene->m_eye, v_dir, 0, 1.f);
                    pr += color.r * weight * 255.f;
                    pg += color.g * weight * 255.f;
                    pb += color.b * weight * 255.f;
                }
            }
            target[ypix * scene->m_width + xpix] = BGRA(pr, pg, pb, 255);
        }
    }
    if(renderCondition != nullptr) {
        printf("Rendered with given condition; %d skipped, %d rendered.\n", skipped, notSkipped);
    }
}

void RayScene::draw(Canvas2D *canvas) {
    int maxSamp = settings.numSuperSamples;
    //printf("drawing\n");
    canvas->resize(m_width, m_height);
    BGRA *data = canvas->data();
    memset(data, 0, m_width*m_height*sizeof(BGRA));
    //assert(canvas->height() == m_height && canvas->width() == m_width);
    // turns out multithreading this is pretty easy

    int nsamps = 1;
    if(settings.useSuperSampling && !settings.useAntiAliasing) // SS uses max num every time
        nsamps = maxSamp;
    int nthreads = settings.useMultiThreading ? 16 : 1;
    int rows_per = m_height / nthreads;
    int extra_rows = m_height % nthreads;
    int cur_row = 0;
    std::thread threads[nthreads];
    // launch all the threads
    printf("Starting rendering with %d threads\n", nthreads);
    fflush(stdout);
    double start = get_time();
    for(int i = 0; i < nthreads; i++) {
        int nrows = rows_per + (i < extra_rows ? 1 : 0);
        threads[i] = std::thread(renderWithParams, this, data, cur_row, nrows, nsamps, nullptr);
        cur_row += nrows;
    }
    assert(cur_row == m_height);
    // now wait for all to finish
    for(int i = 0; i < nthreads; i++) {
        threads[i].join();
    }
    if(settings.useAntiAliasing) {
    }
    printf("Rendering done, took %f secs\n", get_time() - start);
    fflush(stdout);

    canvas->update();
}

RayScene::~RayScene()
{

}

