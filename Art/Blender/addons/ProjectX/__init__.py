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

from mathutils import Matrix, Vector

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
						out.write('\t\t\t\t<uv Value="%f,%f"/>\n' % (data.uv_layers.active.data[loopIndex].uv[:]));
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