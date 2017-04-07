// uint64_t const NO = -1;

#include <unordered_map>
#include <memory>
#include <limits>
#include <stdint.h>
#include <functional>


template<uint32_t S>
class AbstractVEBTree {
public:
  virtual void add(uint64_t) = 0;
  virtual void remove(uint64_t) = 0;
  virtual uint64_t next(uint64_t)= 0;
  virtual uint64_t prev(uint64_t)= 0;
  virtual uint64_t getMin() const = 0;
  virtual uint64_t getMax() const = 0;
};

template <uint32_t S>
struct VEBTree : AbstractVEBTree<S>
{
private:
    constexpr static uint64_t none = std::numeric_limits<uint64_t>::max();
    constexpr static uint32_t child_size = ((S + 1) / 2);
    std::unordered_map<uint64_t, std::shared_ptr<VEBTree<child_size>>> children;
    std::shared_ptr<VEBTree<child_size>> aux;
    uint64_t _min, _max;

    inline static uint64_t high(uint64_t x)
    {
        return x >> child_size;
    }

    inline static uint64_t low(uint64_t x)
    {
        return x & ((1 << child_size) - 1);
    }

    inline static uint64_t merge(uint64_t high, uint64_t low)
    {
        return low + (high << child_size);
    }

    uint64_t common(std::function<uint64_t (VEBTree<child_size>*)> &&minmax,
                    std::function<uint64_t (VEBTree<child_size>*)> &&maxmin,
                    std::function<uint64_t(VEBTree<child_size>*, uint64_t)> &&nextprev,
                    std::function<bool (uint64_t, uint64_t)> &&cmpr, uint64_t getMinMax, uint64_t x)
    {
        uint64_t _high = high(x), _low = low(x);
        if (children.find(_high) != children.end() && !children[_high]->empty()
            && cmpr(minmax(children[_high].get()), _low))
            return merge(_high, nextprev(children[_high].get(), _low));
        uint64_t _np = nextprev(aux.get(), _high);
        if (_np == none)
            return getMinMax;
        return merge(_np, maxmin(children[_np].get()));
    }

public:
    VEBTree() : _min(none)
    {

        if (S > 1)
            aux = std::make_shared<VEBTree<child_size>>();
    }

    inline bool empty() const
    {
        return _min == none;
    }

    inline uint64_t getMin() const
    {
        return _min;
    }

    inline uint64_t getMax() const
    {
        return _max;
    }

    void add(uint64_t x)
    {
        if (empty())
        {
            _min = _max = x;
            return;
        }
        if (x == _min || x == _max)
            return;
        if (_min == _max)
        {
            if (x < _min)
                _min = x;
            else
                _max = x;
            return;
        }
        if (x < _min)
            std::swap(x, _min);
        if (x > _max)
            std::swap(x, _max);
        if (S != 1)
        {
            uint64_t _high = high(x), _low = low(x);
            if (children.find(_high) == children.end())
            {
                children[_high] = std::make_shared<VEBTree<child_size>>();
            }
            if (children[_high]->empty())
            {
                aux->add(_high);
            }
            children[_high]->add(_low);
        }
    }

    void remove(uint64_t x)
    {
        if (_min == _max && _min == x)
        {
            _min = none;
            return;
        }
        bool aux_emtpy = (S == 1) || aux->empty();
        if (_min == x)
        {
            if (aux_emtpy)
            {
                _min = _max;
                return;
            }
            x = merge(aux->getMin(), children[aux->getMin()]->getMin());
            _min = x;
        }
        else if (_max == x)
        {
            if (aux_emtpy)
            {
                _max = _min;
                return;
            }
            x = merge(aux->getMax(), children[aux->getMax()]->getMax());
            _max = x;
        }
        if (aux_emtpy)
            return;
        uint64_t _high = high(x), _low = low(x);
        children[_high]->remove(_low);
        if (children[_high]->empty())
            aux->remove(_high);
    }

    uint64_t next(uint64_t x)
    {
        if (empty() || _max <= x)
            return none;
        if (_min > x)
            return _min;
        if (S == 1 || aux -> empty())
            return _max;
        return common(&VEBTree<child_size>::getMax, &VEBTree<child_size>::getMin, &VEBTree<child_size>::next,
                      [](uint64_t a, uint64_t b) {return a > b;}, _max, x);
    }

    uint64_t prev(uint64_t x)
    {
        if (empty() || _min >= x)
            return none;
        if (x > _max)
            return _max;
        if (S == 1 || aux->empty())
            return _min;
        return common(&VEBTree<child_size>::getMin, &VEBTree<child_size>::getMax, &VEBTree<child_size>::prev,
                      [](uint64_t a, uint64_t b) {return a < b;}, _min, x);
    }

};
