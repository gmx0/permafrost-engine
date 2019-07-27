/*
 *  This file is part of Permafrost Engine. 
 *  Copyright (C) 2019 Eduard Permyakov 
 *
 *  Permafrost Engine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Permafrost Engine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Linking this software statically or dynamically with other modules is making 
 *  a combined work based on this software. Thus, the terms and conditions of 
 *  the GNU General Public License cover the whole combination. 
 *  
 *  As a special exception, the copyright holders of Permafrost Engine give 
 *  you permission to link Permafrost Engine with independent modules to produce 
 *  an executable, regardless of the license terms of these independent 
 *  modules, and to copy and distribute the resulting executable under 
 *  terms of your choice, provided that you also meet, for each linked 
 *  independent module, the terms and conditions of the license of that 
 *  module. An independent module is a module which is not derived from 
 *  or based on Permafrost Engine. If you modify Permafrost Engine, you may 
 *  extend this exception to your version of Permafrost Engine, but you are not 
 *  obliged to do so. If you do not wish to do so, delete this exception 
 *  statement from your version.
 *
 */

#include "render_gl.h"
#include "mesh.h"
#include "texture.h"
#include "vertex.h"
#include "shader.h"
#include "gl_assert.h"
#include "gl_uniforms.h"
#include "public/render.h"
#include "../main.h"
#include "../map/public/tile.h"
#include "../map/public/map.h"

#include <assert.h>
#include <string.h>


struct render_water_ctx{
    struct mesh    surface;
    struct texture dudv;
    struct texture normal;
};

#define WATER_LVL       (-1.0f * Y_COORDS_PER_TILE + 2.0f)
#define ARR_SIZE(a)     (sizeof(a)/sizeof(a[0])) 
#define DUDV_PATH       "assets/water_textures/dudvmap.png"
#define NORM_PATH       "assets/water_textures/normalmap.png"

/*****************************************************************************/
/* STATIC VARIABLES                                                          */
/*****************************************************************************/

static struct render_water_ctx s_ctx;

/*****************************************************************************/
/* EXTERN FUNCTIONS                                                          */
/*****************************************************************************/

bool R_GL_WaterInit(void)
{
    bool ret = true;

    ret = R_Texture_Load(g_basepath, DUDV_PATH, &s_ctx.dudv.id);
    if(!ret)
        goto fail_dudv;
    s_ctx.dudv.tunit = GL_TEXTURE0;
    
    ret = R_Texture_Load(g_basepath, NORM_PATH, &s_ctx.normal.id);
    if(!ret)
        goto fail_normal;
    s_ctx.normal.tunit = GL_TEXTURE1;

    const vec3_t tl = (vec3_t){+1.0f, WATER_LVL, +1.0f};
    const vec3_t tr = (vec3_t){-1.0f, WATER_LVL, +1.0f};
    const vec3_t bl = (vec3_t){+1.0f, WATER_LVL, -1.0f};
    const vec3_t br = (vec3_t){-1.0f, WATER_LVL, -1.0f};

    vec3_t vbuff[] = {
        tl, bl, tr,
        bl, br, tr
    };

    glGenVertexArrays(1, &s_ctx.surface.VAO);
    glBindVertexArray(s_ctx.surface.VAO);

    glGenBuffers(1, &s_ctx.surface.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, s_ctx.surface.VBO);
    glBufferData(GL_ARRAY_BUFFER, ARR_SIZE(vbuff) * sizeof(struct vertex), vbuff, GL_STATIC_DRAW);
    s_ctx.surface.num_verts = ARR_SIZE(vbuff);

    /* Attribute 0 - position */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3_t), (void*)0);
    glEnableVertexAttribArray(0);

    GL_ASSERT_OK();
    return ret;

fail_normal:
    R_Texture_Free(DUDV_PATH);
fail_dudv:
    return ret;
}

void R_GL_WaterShutdown(void)
{
    assert(s_ctx.dudv.id > 0);
    assert(s_ctx.normal.id > 0);
    assert(s_ctx.surface.VBO > 0);
    assert(s_ctx.surface.VAO > 0);

    R_Texture_Free(DUDV_PATH);
    R_Texture_Free(NORM_PATH);

    glDeleteBuffers(1, &s_ctx.surface.VAO);
    glDeleteBuffers(1, &s_ctx.surface.VBO);
    memset(&s_ctx, 0, sizeof(s_ctx));
}

void R_GL_DrawWater(const struct map *map)
{
    GLuint shader_prog = R_Shader_GetProgForName("mesh.static.colored");
    glUseProgram(shader_prog);

    vec4_t blue = (vec4_t){0.0f, 0.0f, 1.0f, 1.0f};

    GLuint loc = glGetUniformLocation(shader_prog, GL_U_COLOR);
    glUniform4fv(loc, 1, blue.raw);

    R_Texture_GL_Activate(&s_ctx.dudv, shader_prog);
    R_Texture_GL_Activate(&s_ctx.normal, shader_prog);

    vec3_t pos = M_GetCenterPos(map);
    mat4x4_t trans;
    PFM_Mat4x4_MakeTrans(pos.x, pos.y, pos.z, &trans);

    struct map_resolution res;
    M_GetResolution(map, &res);
    float half_x = (res.chunk_w * res.tile_w * X_COORDS_PER_TILE) / 2.0f;
    float half_z = (res.chunk_h * res.tile_h * Z_COORDS_PER_TILE) / 2.0f;

    mat4x4_t scale;
    PFM_Mat4x4_MakeScale(half_x, 1.0f, half_z, &scale);

    mat4x4_t model;
    PFM_Mat4x4_Mult4x4(&trans, &scale, &model);

    loc = glGetUniformLocation(shader_prog, GL_U_MODEL);
    glUniformMatrix4fv(loc, 1, GL_FALSE, model.raw);

    glBindVertexArray(s_ctx.surface.VAO);
    glDrawArrays(GL_TRIANGLES, 0, s_ctx.surface.num_verts);
    GL_ASSERT_OK();
}

