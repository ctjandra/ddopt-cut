/**
 * Domain for binary problems
 */

#include "bp_domains.hpp"


void BPDomains::init(int _nvars)
{
	nvars = _nvars;
	nvars_set_zero = 0;
	nvars_set_one = 0;
	domains.resize(nvars + 2);
	for (int i = 0; i < nvars; ++i) {
		domains[i].domain = DOM_ZERO_ONE;
		domains[i].var = i;
	}
	int start = start_index();
	int end = end_index();
	domains[start].domain = DOM_UNDEFINED;
	domains[start].var = start;
	domains[start].next_set = end;
	domains[end].domain = DOM_UNDEFINED;
	domains[end].var = end;
	domains[end].prev_set = start;

	// Update pointers to add node to unset and unproc
	for (int i = 0; i < nvars - 1; ++i) {
		domains[i].next_unset = i+1;
		domains[i].next_unproc = i+1;
		domains[i+1].prev_unset = i;
		domains[i+1].prev_unproc = i;
	}

	// Update pointers for start and end
	domains[start].next_unset = 0;
	domains[start].next_unproc = 0;
	domains[0].prev_unset = start;
	domains[0].prev_unproc = start;
	domains[end].prev_unset = nvars - 1;
	domains[end].prev_unproc = nvars - 1;
	domains[nvars-1].next_unset = end;
	domains[nvars-1].next_unproc = end;

	// assert(consistent_all());
}

BPDomains::BPDomains(const BPDomains& rhs) : BPDomains()
{
	nvars = rhs.nvars;
	domains = rhs.domains;
	nvars_set_zero = rhs.nvars_set_zero;
	nvars_set_one = rhs.nvars_set_one;
}

BPDomains& BPDomains::operator=(const BPDomains& rhs)
{
	nvars = rhs.nvars;
	domains = rhs.domains;
	nvars_set_zero = rhs.nvars_set_zero;
	nvars_set_one = rhs.nvars_set_one;
	return *this;
}

void BPDomains::set_domain(int i, BPDomain dom)
{
	if (domains[i].domain == dom) {
		return;
	}

	assert(domains[i].domain != DOM_PROCESSED); // Processed domain cannot be reverted
	assert(!(domains[i].domain == DOM_ONE && dom == DOM_ZERO));
	assert(!(domains[i].domain == DOM_ZERO && dom == DOM_ONE)); // Domain can only be either restricted or relaxed

	if (dom == DOM_ONE || dom == DOM_ZERO) {
		// DOM_ZERO_ONE to DOM_ONE or DOM_ZERO, assuming above asserts satisfied
		remove_unset(i);
		add_set(i);

		if (dom == DOM_ONE) {
			nvars_set_one++;
		} else { // dom == DOM_ZERO
			nvars_set_zero++;
		}

	} else if (dom == DOM_ZERO_ONE) {
		// DOM_ONE or DOM_ZERO to DOM_ZERO_ONE, assuming above asserts satisfied
		remove_set(i);
		add_unset(i);

		if (domains[i].domain == DOM_ONE) {
			nvars_set_one--;
		} else { // domains[i].domain == DOM_ZERO
			nvars_set_zero--;
		}

	} else if (dom == DOM_PROCESSED) {
		// Any domain to DOM_PROCESSED

		remove_unprocessed(i);
		if (domains[i].domain == DOM_ZERO_ONE) {
			remove_unset(i);
		} else {
			remove_set(i);

			if (domains[i].domain == DOM_ONE) {
				nvars_set_one--;
			} else { // domains[i].domain == DOM_ZERO
				nvars_set_zero--;
			}
		}
	}

	assert(nvars_set_zero >= 0);
	assert(nvars_set_one >= 0);

	domains[i].domain = dom;

	// assert(consistent_all());
}

void BPDomains::add_set(int i)
{
	int end = end_index();
	BPDomainNode& node = domains[i];
	assert(node.domain != DOM_ZERO && node.domain != DOM_ONE);
	assert(node.prev_set == BP_DOMAIN_NODE_NULL && node.next_set == BP_DOMAIN_NODE_NULL);
	node.prev_set = domains[end].prev_set;
	node.next_set = end;
	domains[domains[end].prev_set].next_set = i;
	domains[end].prev_set = i;
}

void BPDomains::add_unset(int i)
{
	int end = end_index();
	BPDomainNode& node = domains[i];
	assert(node.domain != DOM_ZERO_ONE);
	assert(node.prev_unset == BP_DOMAIN_NODE_NULL && node.next_unset == BP_DOMAIN_NODE_NULL);
	node.prev_unset = domains[end].prev_unset;
	node.next_unset = end;
	domains[domains[end].prev_unset].next_unset = i;
	domains[end].prev_unset = i;
}

void BPDomains::remove_set(int i)
{
	BPDomainNode& node = domains[i];
	assert(node.domain == DOM_ZERO || node.domain == DOM_ONE);
	assert(node.prev_set != BP_DOMAIN_NODE_NULL && node.next_set != BP_DOMAIN_NODE_NULL);
	domains[node.prev_set].next_set = node.next_set;
	domains[node.next_set].prev_set = node.prev_set;
	node.prev_set = BP_DOMAIN_NODE_NULL;
	node.next_set = BP_DOMAIN_NODE_NULL;
}

void BPDomains::remove_unset(int i)
{
	BPDomainNode& node = domains[i];
	assert(node.domain == DOM_ZERO_ONE);
	assert(node.prev_unset != BP_DOMAIN_NODE_NULL && node.next_unset != BP_DOMAIN_NODE_NULL);
	domains[node.prev_unset].next_unset = node.next_unset;
	domains[node.next_unset].prev_unset = node.prev_unset;
	node.prev_unset = BP_DOMAIN_NODE_NULL;
	node.next_unset = BP_DOMAIN_NODE_NULL;
}

void BPDomains::remove_unprocessed(int i)
{
	BPDomainNode& node = domains[i];
	assert(node.domain != DOM_PROCESSED);
	assert(node.prev_unproc != BP_DOMAIN_NODE_NULL && node.next_unproc != BP_DOMAIN_NODE_NULL);
	domains[node.prev_unproc].next_unproc = node.next_unproc;
	domains[node.next_unproc].prev_unproc = node.prev_unproc;
	node.prev_unproc = BP_DOMAIN_NODE_NULL;
	node.next_unproc = BP_DOMAIN_NODE_NULL;
}
