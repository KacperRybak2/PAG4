#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Model.h>

enum ObjectType{Other, Wall, Roof, Root, UsingSkyBox, Houses};

class Object
{
private:
	bool isDirty = true;
	Model* model;
	Shader* shader;

	glm::mat4 getRorationMatrixFromVector(glm::vec3 vector) {
		glm::mat4 matrix(1);
		matrix = glm::rotate(matrix, glm::radians(vector.x), glm::vec3(1.0f, 0.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(vector.y), glm::vec3(0.0f, 1.0f, 0.0f));
		matrix = glm::rotate(matrix, glm::radians(vector.z), glm::vec3(0.0f, 0.0f, 1.0f));
		return matrix;
	}

	glm::mat4 getUpdatedModelMatrix() {
		glm::mat4 R_around_origin = getRorationMatrixFromVector(rotationAroundOrigin);
		glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 R = getRorationMatrixFromVector(rotation);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
		return R_around_origin * T * R * rotationMatrix * S;
	}
public:
	glm::vec3 rotationAroundOrigin;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::mat4 rotationMatrix = glm::mat4(1);
	glm::vec3 scale;
	glm::mat4 modelMatrix;
	glm::mat4 modelMatrixWithoutScaleAndRotation;
	std::vector<Object*> children;
	ObjectType objectType = Other;
	bool drawable = true;
	static unsigned int cubemapTexture;

	Object(Model* model, Shader* shader, ObjectType ot = Other, glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1), glm::vec3 rotationAroundOrigin = glm::vec3(0)) :
			model(model), shader(shader), objectType(ot), position(position), rotation(rotation), scale(scale), rotationAroundOrigin(rotationAroundOrigin), modelMatrix(glm::mat4(1)) {}
	Object(glm::vec3 position = glm::vec3(0), glm::vec3 rotation = glm::vec3(0), glm::vec3 scale = glm::vec3(1), glm::vec3 rotationAroundOrigin = glm::vec3(0)) :
			position(position), rotation(rotation), scale(scale), rotationAroundOrigin(rotationAroundOrigin), modelMatrix(glm::mat4(1)) {
		drawable = false;
	}

	void set_scale(float x, float y, float z) {
		scale = glm::vec3(x, y, z);
		isDirty = true;
	}

	void rotate(float x, float y, float z) {
		rotation = glm::vec3(x, y, z);
		isDirty = true;
	}
	void rotate(glm::vec3 r) {
		rotation = r;
		isDirty = true;
	}

	void set_position(float x, float y, float z) {
		position = glm::vec3(x, y, z);
		isDirty = true;
	}
	void set_position(glm::vec3 p) {
		position = p;
		isDirty = true;
	}

	void rotateAroundOrigin(float x, float y, float z) {
		rotationAroundOrigin = glm::vec3(x, y, z);
		isDirty = true;
	}
	void rotateAroundOrigin(glm::vec3 r) {
		rotationAroundOrigin = r;
		isDirty = true;
	}

	void add_child(Object* child) {
		children.push_back(child);
	}
	void setRotationMatrix(glm::mat4 rm) {
		rotationMatrix = rm;
		isDirty = true;
	}

	virtual void updateAllChildrenTransforms(const glm::mat4* parentTransform = &glm::mat4(1), bool isDirty = false) {
		bool _isDirty = this->isDirty | isDirty;
		if (_isDirty) {
			modelMatrix = getUpdatedModelMatrix();
			modelMatrix = *parentTransform * modelMatrix;
			this->isDirty = false;
			modelMatrixWithoutScaleAndRotation = *parentTransform * getRorationMatrixFromVector(rotationAroundOrigin) * glm::translate(glm::mat4(1.0f), position) * getRorationMatrixFromVector(rotation);
		}
		
		if (objectType != Houses) {
			for (auto& child : children) {
				child->updateAllChildrenTransforms(&modelMatrixWithoutScaleAndRotation, _isDirty);
			}
		}
	}
	void drawAllObjects(glm::mat4 projection, glm::mat4 view) {
		if (objectType == Other && drawable) {
			shader->use();
			shader->setMat4("model", modelMatrix);
			shader->setMat4("view", view);
			shader->setMat4("projection", projection);
			model->Draw(*shader);
		}
		else if (objectType == UsingSkyBox && drawable) {
			shader->use();
			shader->setMat4("model", modelMatrix);
			shader->setMat4("view", view);
			shader->setMat4("projection", projection);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			model->Draw(*shader);
		}
		if (objectType != Houses) {
			for (auto& child : children) {
				child->drawAllObjects(projection, view);
			}
		}
	}




	///////////////////////////////////////////////////
	std::vector<glm::mat4> getAllMatricesOfType(ObjectType ot) {
		std::vector<glm::mat4> matrices;
		if (objectType == ot) {
			matrices.push_back(modelMatrix);
		}
		for (auto& child : children) {
			std::vector<glm::mat4> tmp;
			tmp = child->getAllMatricesOfType(ot);
			matrices.insert(matrices.end(), tmp.begin(), tmp.end());
		}
		return matrices;
	}
};

