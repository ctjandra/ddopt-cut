/**
 * Functions to help with a simple top-down or bottom-up pass through a BDD
 */

#include <cassert>
#include "bdd_pass.hpp"


void bdd_pass(BDD* bdd, BDDPassFunc* top_down, BDDPassFunc* bottom_up)
{
	BDDPassValues* source_val;
	BDDPassValues* target_val;
	int bdd_size = bdd->layers.size();

	if (top_down == NULL && bottom_up == NULL) {
		cout << "Warning: Top-down/bottom-up pass attempted without required functions" << endl;
		return; // Nothing needs to be done
	}

	if (!bdd->constructed) {
		// A pass can only be done after the BDD is constructed, not during
		cout << "Error: Top-down/bottom-up pass attempted during DD construction" << endl;
		return;
	}

	assert(bdd->layers.size() > 0);

	int root_layer = bdd->get_root_layer();
	int terminal_layer = bdd->get_terminal_layer();
	assert(bdd->layers[root_layer].size() == 1);
	assert(bdd->layers[terminal_layer].size() == 1);

	// Initialize auxiliary variables
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			BDDPassValues* pass_val = new BDDPassValues();
			if (top_down != NULL) {
				pass_val->top_down_val = top_down->init_val();
			}
			if (bottom_up != NULL) {
				pass_val->bottom_up_val = bottom_up->init_val();
			}
			bdd->layers[layer][k]->temp_data = pass_val;
		}
	}

	// Top-down pass
	if (top_down != NULL) {
		boost::any_cast<BDDPassValues*>(bdd->layers[root_layer][0]->temp_data)->top_down_val = top_down->start_val();

		for (int layer = 0; layer < bdd_size; ++layer) {
			int size = bdd->layers[layer].size();
			for (int k = 0; k < size; ++k) {
				Node* source = bdd->layers[layer][k];
				source_val = boost::any_cast<BDDPassValues*>(source->temp_data); // parent (source)

				if (source->zero_arc != NULL) {
					target_val = boost::any_cast<BDDPassValues*>(source->zero_arc->temp_data);
					target_val->top_down_val = top_down->apply(layer, bdd->layer_to_var[layer], 0,
					                           source_val->top_down_val, target_val->top_down_val, source, source->zero_arc);
				}

				if (source->one_arc != NULL) {
					target_val = boost::any_cast<BDDPassValues*>(source->one_arc->temp_data);
					target_val->top_down_val = top_down->apply(layer, bdd->layer_to_var[layer], 1,
					                           source_val->top_down_val, target_val->top_down_val, source, source->one_arc);
				}
				// cout << "TD: layer " << layer << ", node " << k << ": " << source_val->top_down_val << endl;
				// if (source->zero_arc != NULL) {
				//   cout << "    0-arc to " << source->zero_arc->layer << ", " << source->zero_arc->id << endl;
				// }
				// if (source->zero_arc != NULL) {
				//   cout << "    1-arc to " << source->one_arc->layer << ", " << source->one_arc->id << endl;
				// }
			}
		}
	}

	// Bottom-up pass
	if (bottom_up != NULL) {
		boost::any_cast<BDDPassValues*>(bdd->layers[terminal_layer][0]->temp_data)->bottom_up_val = bottom_up->start_val();

		for (int layer = bdd_size - 1; layer >= 0; --layer) {
			int size = bdd->layers[layer].size();
			for (int k = 0; k < size; ++k) {
				Node* target = bdd->layers[layer][k];
				target_val = boost::any_cast<BDDPassValues*>(target->temp_data); // parent (target)

				if (target->zero_arc != NULL) {
					source_val = boost::any_cast<BDDPassValues*>(target->zero_arc->temp_data);
					target_val->bottom_up_val = bottom_up->apply(layer, bdd->layer_to_var[layer], 0,
					                            source_val->bottom_up_val, target_val->bottom_up_val, target->zero_arc, target);
				}

				if (target->one_arc != NULL) {
					source_val = boost::any_cast<BDDPassValues*>(target->one_arc->temp_data);
					target_val->bottom_up_val = bottom_up->apply(layer, bdd->layer_to_var[layer], 1,
					                            source_val->bottom_up_val, target_val->bottom_up_val, target->one_arc, target);
				}
				// cout << "BU: layer " << layer << ", node " << k << ": " << source_val->bottom_up_val << endl;
				// if (target->zero_arc != NULL) {
				//   cout << "    0-arc to " << target->zero_arc->layer << ", " << target->zero_arc->id << endl;
				// }
				// if (target->zero_arc != NULL) {
				//   cout << "    1-arc to " << target->one_arc->layer << ", " << target->one_arc->id << endl;
				// }
			}
		}
	}
}


void bdd_pass_clean_up(BDD* bdd)
{
	int bdd_size = bdd->layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			delete boost::any_cast<BDDPassValues*>(bdd->layers[layer][k]->temp_data);
			bdd->layers[layer][k]->temp_data = boost::any();
		}
	}
}


void bdd_partial_pass(BDD* bdd, BDDPassFunc* top_down)
{
	BDDPassValues* source_val;
	BDDPassValues* target_val;
	int bdd_size = bdd->layers.size();

	if (top_down == NULL) {
		cout << "Warning: Top-down partial pass attempted without required function" << endl;
		return; // Nothing needs to be done
	}

	// It is expected that the BDD is not fully constructed, but not necessary

	assert(bdd->layers.size() > 0);

	int root_layer = bdd->get_root_layer();
	assert(bdd->layers[root_layer].size() == 1);

	// Initialize auxiliary variables
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* node = bdd->layers[layer][k];
			BDDPassValues* pass_val = new BDDPassValues();
			pass_val->top_down_val = top_down->init_val();
			node->temp_data = pass_val;
			if (node->zero_arc != NULL && node->zero_arc->layer == DD_NODE_ID_OPEN) {
				BDDPassValues* pass_val_zero = new BDDPassValues();
				pass_val_zero->top_down_val = top_down->init_val();
				node->zero_arc->temp_data = pass_val_zero;
			}
			if (node->one_arc != NULL && node->one_arc->layer == DD_NODE_ID_OPEN) {
				BDDPassValues* pass_val_one = new BDDPassValues();
				pass_val_one->top_down_val = top_down->init_val();
				node->one_arc->temp_data = pass_val_one;
			}
		}
	}

	// Top-down pass
	boost::any_cast<BDDPassValues*>(bdd->layers[root_layer][0]->temp_data)->top_down_val = top_down->start_val();

	// Go through entire DD; this is harmless for incomplete DDs since layers not yet constructed have size zero
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* source = bdd->layers[layer][k];
			source_val = boost::any_cast<BDDPassValues*>(source->temp_data); // parent (source)

			if (source->zero_arc != NULL) {
				target_val = boost::any_cast<BDDPassValues*>(source->zero_arc->temp_data);
				target_val->top_down_val = top_down->apply(layer, bdd->layer_to_var[layer], 0,
				                           source_val->top_down_val, target_val->top_down_val, source, source->zero_arc);
			}

			if (source->one_arc != NULL) {
				target_val = boost::any_cast<BDDPassValues*>(source->one_arc->temp_data);
				target_val->top_down_val = top_down->apply(layer, bdd->layer_to_var[layer], 1,
				                           source_val->top_down_val, target_val->top_down_val, source, source->one_arc);
			}
			// cout << "TD: layer " << layer << ", node " << k << ": " << source_val->top_down_val << endl;
			// if (source->zero_arc != NULL) {
			//   cout << "    0-arc to " << source->zero_arc->layer << ", " << source->zero_arc->id << endl;
			// }
			// if (source->zero_arc != NULL) {
			//   cout << "    1-arc to " << source->one_arc->layer << ", " << source->one_arc->id << endl;
			// }
		}
	}
}


void bdd_pass_deep_clean_up(BDD* bdd)
{
	int bdd_size = bdd->layers.size();
	BDDPassValues* null_pass_val = NULL; // NULL casted to BDDPassValues*
	BDDPassValues* pass_val;

	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* node = bdd->layers[layer][k];
			pass_val = boost::any_cast<BDDPassValues*>(node->temp_data);
			if (pass_val != NULL) {
				delete boost::any_cast<BDDPassValues*>(node->temp_data);
			}
			node->temp_data = null_pass_val;
			if (node->zero_arc != NULL && node->zero_arc->layer == DD_NODE_ID_OPEN) {
				pass_val = boost::any_cast<BDDPassValues*>(node->zero_arc->temp_data);
				if (pass_val != NULL) {
					delete boost::any_cast<BDDPassValues*>(node->zero_arc->temp_data);
				}
				node->zero_arc->temp_data = null_pass_val;
			}
			if (node->one_arc != NULL && node->one_arc->layer == DD_NODE_ID_OPEN) {
				pass_val = boost::any_cast<BDDPassValues*>(node->one_arc->temp_data);
				if (pass_val != NULL) {
					delete boost::any_cast<BDDPassValues*>(node->one_arc->temp_data);
				}
				node->one_arc->temp_data = null_pass_val;
			}
		}
	}
}
