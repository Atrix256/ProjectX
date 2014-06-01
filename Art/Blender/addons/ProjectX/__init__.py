bl_info = {
	"name":         "ProjectX Exporters",
	"author":       "Alan Wolfe",
	"blender":      (2,7,0),
	"version":      (0,0,1),
	"location":     "File > Import-Export",
	"description":  "Export custom data formats for ProjectX",
	"category":     "Import-Export"
}
		
import bpy
from bpy_extras.io_utils import ExportHelper

import bpy_extras.io_utils

import os

from mathutils import Matrix, Vector

# flip Y and Z axis, since blender has +Z as up, and we have +Y as up
GLOBAL_EXPORT_MATRIX = Matrix([[1, 0, 0, 0],[0, 0, 1, 0],[0, 1, 0, 0],[0, 0, 0, 1]])

def mesh_triangulate(me):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(me)
    bm.free()

class ExportModel(bpy.types.Operator, ExportHelper):
	bl_idname       = "model.xmd";
	bl_label        = "Save";
	bl_options      = {'PRESET'};
	
	filename_ext    = ".xmd";
	
	def execute(self, context):
		out = open(self.filepath, "w");
		out.write('<?xml version="1.0" encoding="utf-8"?>\n<model>\n')
		for obj in bpy.data.objects:

			try:
				data = obj.to_mesh(bpy.context.scene, True, 'PREVIEW')
			except RuntimeError:
				data = None

			if data is None:
				continue

			data.transform(obj.matrix_world * GLOBAL_EXPORT_MATRIX)

			mesh_triangulate(data)
			data.calc_tessface();
			data.calc_tangents();

			out.write('\t<object>\n')

			source_dir = os.path.dirname(bpy.data.filepath)
			dest_dir = os.path.dirname(bpy.data.filepath)

			if len(data.materials) == 0:
				out.write('\t\t<material/>\n')

			# write materials for this object
			for mat_num, mat in enumerate(data.materials):
				out.write('\t\t<material>\n')

				# todo: EmissiveColor
				# todo: ReflectionAmount
				# todo: Absorbance

				out.write('\t\t\t<DiffuseColor Value="%f,%f,%f"/>\n' % (mat.diffuse_intensity * mat.diffuse_color)[:])

				# write images
				image_map = {}
				# backwards so topmost are highest priority
				for mtex in reversed(mat.texture_slots):
					if mtex and mtex.texture and mtex.texture.type == 'IMAGE':
						image = mtex.texture.image
						if image:
							# texface overrides others
							if      (mtex.use_map_color_diffuse and
									(mtex.use_map_warp is False) and
									(mtex.texture_coords != 'REFLECTION')):
								image_map["DiffuseTexture"] = image
							if mtex.use_map_ambient:
								image_map["AmbientTexture_Unused"] = image
							# this is the Spec intensity channel but Ks stands for specular Color
							'''
							if mtex.use_map_specular:
								image_map["SpecularTexture_Unused"] = image
							'''
							if mtex.use_map_color_spec:  # specular color
								image_map["SpecularTexture_Unused"] = image
							if mtex.use_map_hardness:  # specular hardness/glossiness
								image_map["HardnessTexture_Unused"] = image
							if mtex.use_map_alpha:
								image_map["AlphaTexture_Unused"] = image
							if mtex.use_map_translucency:
								image_map["TranslucencyTexture_Unused"] = image
							if mtex.use_map_normal and (mtex.texture.use_normal_map is True):
								image_map["BumpTexture_Unused"] = image
							if mtex.use_map_normal and (mtex.texture.use_normal_map is False):
								image_map["NormalTexture"] = image                      
							if mtex.use_map_color_diffuse and (mtex.texture_coords == 'REFLECTION'):
								image_map["ReflectionTexture_Unused"] = image
							if mtex.use_map_emit:
								image_map["EmissiveTexture"] = image

				for key, image in image_map.items():
					out.write('\t\t\t<%s Value="%s"/>\n' % (key, repr(image.filepath)[1:-1]))

				out.write('\t\t\t<SpecularColor Value="%f,%f,%f"/>\n' % (mat.specular_intensity * mat.specular_color)[:])

				# convert from blenders spec to 0 - 1000 range.
				if mat.specular_shader == 'WARDISO':
					tspec = (0.4 - mat.specular_slope) / 0.0004
				else:
					tspec = (mat.specular_hardness - 1) * 1.9607843137254901
				out.write('\t\t\t<SpecularPower Value="%f"/>\n' % tspec)

				if hasattr(mat, "ior"):
					out.write('\t\t\t<RefractionIndex Value="%f"/>\n' % mat.ior)

				out.write('\t\t\t<RefractionAmount Value="%f"/>\n' % (1.0 - mat.alpha))

				out.write('\t\t</material>\n')

			# write polygons
			for face_num, face in enumerate(data.polygons):
				out.write('\t\t<face>\n')

				for loopIndex in (face.loop_indices):
					vert = data.vertices[data.loops[loopIndex].vertex_index];
					out.write('\t\t\t<vert>\n');
					out.write('\t\t\t\t<pos Value="%f,%f,%f"/>\n' % (vert.co[:]));
					out.write('\t\t\t\t<normal Value="%f,%f,%f"/>\n' % (data.loops[loopIndex].normal[:]));
					out.write('\t\t\t\t<tangent Value="%f,%f,%f"/>\n' % (data.loops[loopIndex].tangent[:]));
					out.write('\t\t\t\t<bitangent Value="%f,%f,%f"/>\n' % (data.loops[loopIndex].bitangent[:]));
					if data.uv_layers.active != None:
						out.write('\t\t\t\t<uv Value="%f,%f"/>\n' % (data.uv_layers.active.data[loopIndex].uv.x, 1.0 - data.uv_layers.active.data[loopIndex].uv.y));
					out.write('\t\t\t</vert>\n');

				out.write('\t\t</face>\n')
			out.write('\t</object>\n')
			bpy.data.meshes.remove(data)
		out.write('</model>\n')
		out.close();
		return {'FINISHED'};

def model_menu_func(self, context):
	self.layout.operator(ExportModel.bl_idname, text="ProjectX Model(.xmd)");

def register():
	bpy.utils.register_module(__name__);
	bpy.types.INFO_MT_file_export.append(model_menu_func);
	
def unregister():
	bpy.utils.unregister_module(__name__);
	bpy.types.INFO_MT_file_export.remove(model_menu_func);

if __name__ == "__main__":
	register()