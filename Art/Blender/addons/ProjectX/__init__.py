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

class ExportModel(bpy.types.Operator, ExportHelper):
	bl_idname       = "model.xmd";
	bl_label        = "Save";
	bl_options      = {'PRESET'};
	
	filename_ext    = ".xmd";
	
	def execute(self, context):
		out = open(self.filepath, "w");
		out.write('<model>\n')
		for obj in bpy.data.objects:
			data = obj.to_mesh(bpy.context.scene, True, 'PREVIEW')
			data.calc_tessface()
			out.write('\t<object>\n')
			for face_num, face in enumerate(data.tessfaces):
				out.write('\t\t<face>\n')
				for vert_num in (face.vertices):
					vert = data.vertices[vert_num]
					out.write( '\t\t\t<vert>%f %f %f</vert>\n' % (vert.co.x, vert.co.y, vert.co.z) )
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