#include "material.hh"

#include "../log.hh"

#include <stdlib.h>
#include <yyjson.h>

namespace vkb::vk
{
	namespace
	{
		void read_struct(yyjson_val* j_fields, property_layout_object* obj)
		{
			yyjson_arr_iter j_fields_iter;
			yyjson_arr_iter_init(j_fields, &j_fields_iter);
			yyjson_val* j_field;

			while ((j_field = yyjson_arr_iter_next(&j_fields_iter)))
			{
				mc::string name(yyjson_get_str(yyjson_obj_get(j_field, "name")));
				j_field = yyjson_obj_get(j_field, "type");
				char const* type_str = yyjson_get_str(yyjson_obj_get(j_field, "kind"));
				if (strcmp(type_str, "struct") == 0)
				{
					property_layout_object* child_obj = new property_layout_object;
					child_obj->name = static_cast<mc::string&&>(name);
					read_struct(yyjson_obj_get(j_field, "fields"), child_obj);
					obj->props.emplace_back(child_obj);
				}
				else if (strcmp(type_str, "matrix") == 0)
				{
					yyjson_val* j_mat_type = yyjson_obj_get(j_field, "elementType");
					if (yyjson_get_uint(yyjson_obj_get(j_field, "rowCount")) == 4 &&
					    yyjson_get_uint(yyjson_obj_get(j_field, "columnCount")) == 4 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_mat_type, "kind")),
					           "scalar") == 0 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_mat_type, "scalarType")),
					           "float32") == 0)
					{
						property_layout_simple* child_mat = new property_layout_simple;
						child_mat->name = static_cast<mc::string&&>(name);
						child_mat->type = property_layout_simple::data_type::float44;
						obj->props.emplace_back(child_mat);
					}
				}
				else if (strcmp(type_str, "resource") == 0)
				{
					property_layout_resource* child_res = new property_layout_resource;
					child_res->name = static_cast<mc::string&&>(name);

					char const* res_type_str =
						yyjson_get_str(yyjson_obj_get(j_field, "baseShape"));
					if (strcmp(res_type_str, "texture2D") == 0)
						child_res->type =
							property_layout_resource::resource_type::texture2D;

					yyjson_val* j_resource_result = yyjson_obj_get(j_field, "resultType");
					yyjson_val* j_type = yyjson_obj_get(j_resource_result, "elementType");
					if (yyjson_get_uint(
							yyjson_obj_get(j_resource_result, "elementCount")) == 4 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_type, "kind")),
					           "scalar") == 0 &&
					    strcmp(yyjson_get_str(yyjson_obj_get(j_type, "scalarType")),
					           "float32") == 0)
					{
						child_res->format =
							property_layout_resource::resource_format::float4;
					}

					yyjson_val* j_binding = yyjson_obj_get(j_field, "binding");
					child_res->binding =
						yyjson_get_uint(yyjson_obj_get(j_binding, "index"));
					obj->props.emplace_back(child_res);
				}
				else
				{
					// TODO
				}
			}
		}

		void read_param(yyjson_val* j_param, mc::vector<set_layout>& sets)
		{
			yyjson_val* j_binding = yyjson_obj_get(j_param, "binding");
			if (strcmp(yyjson_get_str(yyjson_obj_get(j_binding, "kind")),
			           "subElementRegisterSpace") != 0)
				return;

			uint32_t index = yyjson_get_uint(yyjson_obj_get(j_binding, "index"));
			if (sets.size() < index)
				sets.resize(index);

			yyjson_val* type = yyjson_obj_get(j_param, "type");

			// TODO support more sets kind. Currently only supporting ParameterBlock
			// containing struct.
			if (strcmp(yyjson_get_str(yyjson_obj_get(type, "kind")), "parameterBlock") !=
			    0)
				return;

			yyjson_val* j_layout =
				yyjson_obj_get(yyjson_obj_get(type, "elementVarLayout"), "type");
			if (strcmp(yyjson_get_str(yyjson_obj_get(j_layout, "kind")), "struct") != 0)
				return;

			property_layout_object* obj = new property_layout_object;
			obj->name = yyjson_get_str(yyjson_obj_get(j_param, "name"));

			yyjson_val* j_bindings = yyjson_obj_get(j_layout, "bindings");

			uint32_t binding_off {0};
			uint32_t uniform_off {0};
			uint32_t uniform_size {0};

			yyjson_arr_iter elem_bindings_iter;
			yyjson_val*     j_elem_binding;
			yyjson_arr_iter_init(j_bindings, &elem_bindings_iter);
			while ((j_elem_binding = yyjson_arr_iter_next(&elem_bindings_iter)))
			{
				if (strcmp(yyjson_get_str(yyjson_obj_get(j_elem_binding, "kind")),
				           "descriptorTableLayout"))
				{
					binding_off =
						yyjson_get_uint(yyjson_obj_get(j_elem_binding, "index"));
				}
				else if (strcmp(yyjson_get_str(yyjson_obj_get(j_elem_binding, "kind")),
				                "uniform"))
				{
					uniform_off =
						yyjson_get_uint(yyjson_obj_get(j_elem_binding, "offset"));
					uniform_size =
						yyjson_get_uint(yyjson_obj_get(j_elem_binding, "size"));
				}
			}

			read_struct(yyjson_obj_get(j_layout, "fields"), obj);

			sets.emplace_back(obj, uniform_off, uniform_size, binding_off);
		}
	}

	material::material(mc::string_view shader)
	: path_ {shader}
	{
		FILE* file = fopen(path_.data(), "rb");
		log::assert(file != nullptr, "Invalid shader path: %s", path_.data());
		if (file)
			fclose(file);

		mc::string reflect_path;
		reflect_path.reserve(path_.size() + 5);
		reflect_path += path_;
		reflect_path += ".json";

		yyjson_read_err err;
		yyjson_doc*     doc = yyjson_read_file(reflect_path.data(), 0, nullptr, &err);
		if (!doc)
		{
			log::error("Failed to read shader reflection: %s at %ju", err.msg, err.pos);
			return;
		}

		yyjson_val* j_root = yyjson_doc_get_root(doc);

		yyjson_val* j_params = yyjson_obj_get(j_root, "parameters");
		layout_.sets.reserve(yyjson_arr_size(j_params));
		yyjson_arr_iter j_params_iter;
		yyjson_arr_iter_init(j_params, &j_params_iter);
		yyjson_val* j_param;
		while ((j_param = yyjson_arr_iter_next(&j_params_iter)))
			read_param(j_param, layout_.sets);

		// TODO entry points
	}

	bool material::create_pipeline_state()
	{
		/*VkShaderModule shader;
		uint8_t*       shader_buf {nullptr};
		int64_t        shader_size {0};
		FILE*          shader_file {fopen(path_.data(), "rb")};
		if (!shader_file)
		{
		    log::error("Invalid shader path: %s", path_.data());
		    return false;
		}

		fseek(shader_file, 0, SEEK_END);
		fgetpos(shader_file, &shader_size);

		fseek(shader_file, 0, SEEK_SET);
		shader_buf = new uint8_t[shader_size];
		fread(shader_buf, 1, shader_size, shader_file);
		fclose(shader_file);

		VkShaderModuleCreateInfo shader_create_info {};
		shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.codeSize = shader_size;
		shader_create_info.pCode = reinterpret_cast<uint32_t*>(shader_buf);
		// vkCreateShaderModule(device_)
		if (shader_buf)
		    delete[] shader_buf;*/
		return true;
	}
}