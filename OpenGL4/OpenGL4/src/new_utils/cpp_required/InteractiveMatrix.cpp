#include "InteractiveMatrix.h"

namespace nho
{
	using namespace glm;

	/*static*/ sp<ho::Montserrat_BMF> Scalar_3D::font;

	void Scalar_3D::setValue(float inValue)
	{
		value.reset();

		value = inValue; 

		int wholePortion = int(inValue);
		float fractionPortion = inValue - float(wholePortion);

		char textBuffer[128];
		snprintf(textBuffer, sizeof(textBuffer), "%3.3f", inValue);
		symbolicValue = std::string(textBuffer);
		recalculateTextData();
	}

	void Scalar_3D::setValue(const std::string& val)
	{
		value.reset();
		symbolicValue = val;
		recalculateTextData();
	}

	void Scalar_3D::recalculateTextData()
	{
		text3D->wrappedText->text = symbolicValue;
		text3D->wrappedText->refreshState();
		float width = text3D->wrappedText->getLocalWidth();
		float scale = 1.f;
		float widthRatio = width / maxWidth;
		scale = 1 / widthRatio;
		text3D->setLocalScale(vec3(scale));
	}

	////////////////////////////////////////////////////////
	// Scalar_3D
	////////////////////////////////////////////////////////
	void Scalar_3D::render(float dt_sec, const glm::mat4& projection, const glm::mat4& view)
	{
		//glm::mat4 model = text3D->getWorldMat();
		text3D->render(projection, view);
	}

	void Scalar_3D::postConstruct()
	{
		Entity::postConstruct();

		font = !font ? new_sp<ho::Montserrat_BMF>("./assets/textures/font/Montserrat_ss_alpha_1024x1024_wb.png") : font;
		text3D = new_sp<ho::TextBlockSceneNode>(font, "NoValue");
	}


	////////////////////////////////////////////////////////
	// Matrix_3D
	////////////////////////////////////////////////////////



	const sp<Scalar_3D>& Matrix_3D::get(size_t col, size_t row) const
	{
		if (matrix.size() != 0 && col < matrix.size()
			&& matrix[col].size() != 0 && row < matrix[col].size())
		{
			return matrix[col][row];
		}

		return nullScalar;
	}

	void Matrix_3D::set(size_t col, size_t row, const sp<Scalar_3D>& value)
	{
		if (matrix.size() != 0 && col < matrix.size()
			&& matrix[col].size() != 0 && row < matrix[col].size())
		{
			matrix[col][row] = value;
		}
	}

	void Matrix_3D::setSize(size_t inColumn, size_t inRow)
	{
		matrix.resize(inColumn);
		for (size_t col = 0; col < inColumn; ++col)
		{
			matrix[col].resize(inRow);
		}
	}

	void Matrix_3D::setElementBoxSize(float size)
	{
		elementBoxSize = size;
		for (auto& col : matrix)
		{
			for (auto& rowElement : col)
			{
				if (rowElement)
				{
					rowElement->setMaxSize(size);
				}
			}
		}
	}

	void Matrix_3D::render(float dt_sec, const glm::mat4& projection, const glm::mat4& view)
	{

		refreshElements(); //avoid doing this in render tick, adding here for now since doing early set up

		for (size_t colIdx = 0; colIdx < matrix.size(); ++colIdx)
		{
			auto& col = matrix[colIdx];

			for (size_t rowIdx = 0; rowIdx < col.size(); ++rowIdx)
			{
				auto& rowEle = matrix[colIdx][rowIdx];
				rowEle->render(dt_sec, projection, view);
			}
		}

		////////////////////////////////////////////////////////
		// render the scalars 
		////////////////////////////////////////////////////////
	}

	void Matrix_3D::refreshElements()
	{
		float largestSizeSoFar = 0.f;
		size_t largestNumRows = 0;
		size_t largestNumCols = matrix.size();
		for (auto& col : matrix)
		{
			largestNumRows = col.size() > largestNumRows ? col.size() : largestNumRows;
			for (auto& rowEle : col)
			{
				float eleSize = rowEle->getMaxSize();
				largestSizeSoFar = eleSize > largestSizeSoFar ? eleSize : largestSizeSoFar;
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// position and render each scalar now that total sizing is valid
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//NOTE: this is a configuration of local space, where center of matrix is 0,0,0. positioning will be automaticaly handled by scene node.
		float matrixWidthStride = largestSizeSoFar * 2;
		float matrixHeightStride = largestSizeSoFar * 2;
		float matrixWidth = matrixWidthStride * largestNumCols;
		float matrixHeight = matrixWidthStride * largestNumRows;
		const glm::vec3 matrixTopLeft = vec3(-matrixWidth/2.f, matrixHeight/2.f, 0);
		const glm::vec3 scalarStartPoint = matrixTopLeft + -vec3(largestSizeSoFar/2, -largestSizeSoFar/2, 0);
		const glm::vec3 down_n = glm::vec3(0, -1, 0);
		const glm::vec3 right_n = glm::vec3(1, 0, 0);

		size_t col = 0;
		size_t row = 0;
		for (size_t colIdx = 0; colIdx < matrix.size(); ++colIdx)
		{
			auto& col = matrix[colIdx];

			for (size_t rowIdx = 0; rowIdx < col.size(); ++rowIdx)
			{
				auto& rowEle = matrix[colIdx][rowIdx];

				//assuming scalars are centered
				if (!rowEle->hasManualPlacement()) //if user has placed this somewhere for an animation or something, ignore positioning the scalar
				{
					//THIS SHOULD IGNORE MATRIX XFORM -- these are parent/child relationships and will be handled automatically because that's what scene nodes do :)
					//this is entirely a local transform relative to point 0,0,0;
					rowEle->getXform().setLocalPosition(scalarStartPoint + rowIdx * matrixHeightStride*down_n + colIdx * matrixWidthStride*right_n);
				}

				//re parent node if it is not attached and no longer in custom mode
				if (!rowEle->hasManualPlacement() && rowEle->getXform().getParent() != mySceneNode)
				{
					rowEle->getXform().setParent(mySceneNode);
				}
			}
		}
	}

	void Matrix_3D::postConstruct()
	{
		setElementBoxSize(elementBoxSize);

		//todo_fix_scalaring_of_text_on_matrix;
		//todo_scalar_3d_interact_with_clicks_interface; perhaps_matrix_does_distance_check_and_will_slot_it;
		//todo_matrix_placement_of_scalars;
		//todo_matrix_tick;
		//todo_matrix_braket_render_with_line;
		//todo_matrix_matrix_multiply_visual_example;//vectors are just 3x1 multiply special case
		//todo_system_eq_review;
		//todo_set_up_system_eq_review_to_pull_values_from_eq;
	}


}


