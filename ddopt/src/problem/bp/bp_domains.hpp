/**
 * Domain for binary problems
 */

#ifndef BPDOMAINS_HPP_
#define BPDOMAINS_HPP_

#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>

using namespace std;

enum BPDomain {
	DOM_ZERO_ONE = -1,
	DOM_ZERO = 0,
	DOM_ONE = 1,

	DOM_PROCESSED, // Already processed during DD construction; that is, corresponds to a previous layer

	DOM_UNDEFINED // For sentinel nodes
};


/* The structures here allow faster iteration through set, unset, or unprocessed domains.
 * Note that while set and unset iteration is unordered (since variables may be set or unset through branching and merge),
 * unprocessed iteration is ordered since a processed variable never becomes unprocessed again, and this is necessary
 * since we use it for equivalence checks. */

#define BP_DOMAIN_NODE_NULL -1 // must be < 0

/** Node containing domain and structure for traversals */
struct BPDomainNode {
	BPDomain domain;
	int var;

	/* "Pointers" to previous/next nodes that are set (DOM_ZERO or DOM_ONE), unset (DOM_ZERO_ONE),
	 * or unprocessed (all except DOM_PROCESSED).
	 * These are implemented as int since copying states is very frequent and this avoid rearranging pointers to
	 * new nodes in each copy. Profiling suggests that copies could be expensive given its frequency.
	 * Here, ints correspond to indices, and BP_DOMAIN_NODE_NULL corresponds to NULL.
	 */
	int prev_set;
	int prev_unset;
	int prev_unproc;
	int next_set;
	int next_unset;
	int next_unproc;

	BPDomainNode(BPDomain _domain, int _var) : domain(_domain), var(_var),
		prev_set(BP_DOMAIN_NODE_NULL), prev_unset(BP_DOMAIN_NODE_NULL), prev_unproc(BP_DOMAIN_NODE_NULL),
		next_set(BP_DOMAIN_NODE_NULL), next_unset(BP_DOMAIN_NODE_NULL), next_unproc(BP_DOMAIN_NODE_NULL) {}

	BPDomainNode() : BPDomainNode(DOM_UNDEFINED, -1) {}

	BPDomainNode& operator=(const BPDomainNode& rhs)
	{
		domain = rhs.domain;
		var = rhs.var;
		prev_set = rhs.prev_set;
		prev_unset = rhs.prev_unset;
		prev_unproc = rhs.prev_unproc;
		next_set = rhs.next_set;
		next_unset = rhs.next_unset;
		next_unproc = rhs.next_unproc;
		return *this;
	}

	bool operator==(const BPDomainNode& rhs) const
	{
		return (var == rhs.var);
	}

	bool operator!=(const BPDomainNode& rhs) const
	{
		return (var != rhs.var);
	}
};


/** Iterator for set variables */
template <typename Node> // for purposes of defining both non-const and const iterators
class BPDomainsSetIteratorBase : public iterator<forward_iterator_tag, int>
{
	friend class BPDomains;

	int idx;
	const vector<BPDomainNode>* domains;

	BPDomainsSetIteratorBase(int _idx, const vector<BPDomainNode>* _domains) : idx(_idx), domains(_domains) {}

public:

	Node& operator*() const
	{
		return (*domains)[idx];
	}
	const Node* operator->() const
	{
		return &(*domains)[idx];
	}

	const BPDomainsSetIteratorBase& operator++()
	{
		idx = (*domains)[idx].next_set;
		return *this;
	}
	const BPDomainsSetIteratorBase operator++(int)
	{
		BPDomainsSetIteratorBase temp(*this);
		idx = (*domains)[idx].next_set;
		return temp;
	}

	bool operator==(const BPDomainsSetIteratorBase& other) const
	{
		return idx == other.idx;
	}
	bool operator!=(const BPDomainsSetIteratorBase& other) const
	{
		return idx != other.idx;
	}
};

typedef BPDomainsSetIteratorBase<BPDomainNode> BPDomainsSetIterator;
typedef BPDomainsSetIteratorBase<const BPDomainNode> BPDomainsSetConstIterator;


/** Iterator for unset variables */
template <typename Node> // for purposes of defining both non-const and const iterators
class BPDomainsUnsetIteratorBase : public iterator<forward_iterator_tag, int>
{
	friend class BPDomains;

	int idx;
	const vector<BPDomainNode>* domains;

	BPDomainsUnsetIteratorBase(int _idx, const vector<BPDomainNode>* _domains) : idx(_idx), domains(_domains) {}

public:

	Node& operator*() const
	{
		return (*domains)[idx];
	}
	const Node* operator->() const
	{
		return &(*domains)[idx];
	}

	const BPDomainsUnsetIteratorBase& operator++()
	{
		idx = (*domains)[idx].next_unset;
		return *this;
	}
	const BPDomainsUnsetIteratorBase operator++(int)
	{
		BPDomainsUnsetIteratorBase temp(*this);
		idx = (*domains)[idx].next_unset;
		return temp;
	}

	bool operator==(const BPDomainsUnsetIteratorBase& other) const
	{
		return idx == other.idx;
	}
	bool operator!=(const BPDomainsUnsetIteratorBase& other) const
	{
		return idx != other.idx;
	}
};

typedef BPDomainsUnsetIteratorBase<BPDomainNode> BPDomainsUnsetIterator;
typedef BPDomainsUnsetIteratorBase<const BPDomainNode> BPDomainsUnsetConstIterator;


/** Iterator for unprocessed variables */
template <typename Node> // for purposes of defining both non-const and const iterators
class BPDomainsUnprocIteratorBase : public iterator<forward_iterator_tag, int>
{
	friend class BPDomains;

	int idx;
	const vector<BPDomainNode>* domains;

	BPDomainsUnprocIteratorBase(int _idx, const vector<BPDomainNode>* _domains) : idx(_idx), domains(_domains) {}

public:

	Node& operator*() const
	{
		return (*domains)[idx];
	}
	const Node* operator->() const
	{
		return &(*domains)[idx];
	}

	const BPDomainsUnprocIteratorBase& operator++()
	{
		idx = (*domains)[idx].next_unproc;
		return *this;
	}
	const BPDomainsUnprocIteratorBase operator++(int)
	{
		BPDomainsUnprocIteratorBase temp(*this);
		idx = (*domains)[idx].next_unproc;
		return temp;
	}

	bool operator==(const BPDomainsUnprocIteratorBase& other) const
	{
		return idx == other.idx;
	}
	bool operator!=(const BPDomainsUnprocIteratorBase& other) const
	{
		return idx != other.idx;
	}
};

typedef BPDomainsUnprocIteratorBase<BPDomainNode> BPDomainsUnprocIterator;
typedef BPDomainsUnprocIteratorBase<const BPDomainNode> BPDomainsUnprocConstIterator;


/**
 * Essentially a vector of BPDomains, but allows (unordered) iterations over set or unset variables.
 */
class BPDomains
{
public:
	// domains has nvars + 2 elements: it stores start (head) and end (tail) of list for convenience at nvars and nvars+1
	vector<BPDomainNode> domains;
	int nvars;
	int nvars_set_zero;
	int nvars_set_one;

	BPDomains() {}
	BPDomains(const BPDomains& rhs);
	void init(int _nvars);

	~BPDomains() {}

	BPDomain operator[](int i) const
	{
		return domains[i].domain;
	}

	int size()
	{
		return nvars;
	}

	int start_index() const
	{
		return nvars;
	}
	int end_index() const
	{
		return nvars + 1;
	}

	BPDomains& operator=(const BPDomains& rhs);

	/** Set domain of variable i to dom, adjusting pointers in the process */
	void set_domain(int i, BPDomain dom);

	BPDomainsSetIterator begin_set()
	{
		return BPDomainsSetIterator(domains[start_index()].next_set, &domains);
	}
	BPDomainsSetIterator end_set()
	{
		return BPDomainsSetIterator(end_index(), &domains);
	}
	BPDomainsUnsetIterator begin_unset()
	{
		return BPDomainsUnsetIterator(domains[start_index()].next_unset, &domains);
	}
	BPDomainsUnsetIterator end_unset()
	{
		return BPDomainsUnsetIterator(end_index(), &domains);
	}
	BPDomainsUnprocIterator begin_unproc()
	{
		return BPDomainsUnprocIterator(domains[start_index()].next_unproc, &domains);
	}
	BPDomainsUnprocIterator end_unproc()
	{
		return BPDomainsUnprocIterator(end_index(), &domains);
	}

	const BPDomainsSetConstIterator begin_set() const
	{
		return BPDomainsSetConstIterator(domains[start_index()].next_set, &domains);
	}
	const BPDomainsSetConstIterator end_set() const
	{
		return BPDomainsSetConstIterator(end_index(), &domains);
	}
	const BPDomainsUnsetConstIterator begin_unset() const
	{
		return BPDomainsUnsetConstIterator(domains[start_index()].next_unset, &domains);
	}
	const BPDomainsUnsetConstIterator end_unset() const
	{
		return BPDomainsUnsetConstIterator(end_index(), &domains);
	}
	const BPDomainsUnprocConstIterator begin_unproc() const
	{
		return BPDomainsUnprocConstIterator(domains[start_index()].next_unproc, &domains);
	}
	const BPDomainsUnprocConstIterator end_unproc() const
	{
		return BPDomainsUnprocConstIterator(end_index(), &domains);
	}

private:

	void add_set(int i);
	void remove_set(int i);
	void add_unset(int i);
	void remove_unset(int i);
	// add_unprocessed, if implemented, should be used carefully because it may break the order of unprocessed variables
	void remove_unprocessed(int i);

	bool consistent(int i)
	{
		return (((domains[i].prev_set == BP_DOMAIN_NODE_NULL && domains[i].next_set == BP_DOMAIN_NODE_NULL)
		         || (domains[domains[i].next_set].prev_set == i && domains[domains[i].prev_set].next_set == i))
		        && ((domains[i].prev_unset == BP_DOMAIN_NODE_NULL && domains[i].next_unset == BP_DOMAIN_NODE_NULL)
		            || (domains[domains[i].next_unset].prev_unset == i && domains[domains[i].prev_unset].next_unset == i))
		        && ((domains[i].prev_unproc == BP_DOMAIN_NODE_NULL && domains[i].next_unproc == BP_DOMAIN_NODE_NULL)
		            || (domains[domains[i].next_unproc].prev_unproc == i && domains[domains[i].prev_unproc].next_unproc == i)));
	}

	bool consistent_all()
	{
		for (int i = 0; i < nvars; ++i) {
			if (!consistent(i)) {
				return false;
			}
		}
		return true;
	}
};


#endif // BPDOMAINS_HPP
