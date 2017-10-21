/*
 *  Ebml.h
 *  Embedded
 *
 *  Created by Serge on 22/01/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once
#include "base.h"
#include "EbmlTypes.h"
#include "EbmlWriter.h"

namespace ebml {
  /**
   * Abstract definition of an element
   */
  class ElementBase {
  public:
    /**
     * Virtual destructor
     */
    virtual ~ElementBase() {}
    
    /**
     * @return the element id
     */
    id_type id() const;
    /**
     * @return The encoded element outer size
     */
    size_type size() const;
    /**
     * @return the element data size in bytes
     */
    virtual size_type data_size() const = 0;
    ElementBase* next() const;
  protected:
    /**
     * Writes the ebml binary stream
     * @return true if successfull
     */
    virtual bool write(EbmlWriter& writer) const = 0;
    
    ElementBase(id_type id);    
    const id_type m_id;
    ElementBase* m_next;
    
    friend class Master;
  };
  
  /**
   * Element that contains other elements
   */
  class Master: public ElementBase {
  public:
    Master(id_type id);
    /**
     * For a master element, the data size is the sum of the outer size of its sub elements
     */
    virtual size_type data_size() const;
    /**
     * @param element subordinate element to be added. Its lifetime should be equal or exceed
     *        this of the master element
     * @return the master element for chaining appends if needed
     */
    Master& append(ElementBase& element);
    /**
     * @return A pointer to the first subordinate or 0 if none
     */
    ElementBase* sub() const;
    
    /**
     * Writes the ebml binary stream
     * @return true if successfull
     */
    bool write(EbmlWriter& writer) const;
    
  private:
    ElementBase* m_sub;
  };
  
  /**
   * Typed element
   */
  template <typename T>
  class Element: public ElementBase {
  public:
    Element(id_type id);
    Element(id_type id, const T& value);
    virtual size_type data_size() const;
    const T& get_value() const { return m_data; }
    void set_value(const T& value) { m_data = value; }
  protected:
    /**
     * Writes the ebml binary stream
     * @return true if successfull
     */
    bool write(EbmlWriter& writer) const;

  private:
    T m_data;
  };
    
  /**
   * Typed element, stores the reference of a value.
   * The value lifetime must be greater than that of the element.
   */
  template <typename T>
  class ElementRef: public ElementBase {
  public:
    ElementRef(id_type id, T& value);
    virtual size_type data_size() const;
    const T& get_value() const { return m_data; }
    T& get_ref() const { return m_data; }
    void set_value(T& value) { m_data = value; }
  protected:
    /**
     * Writes the ebml binary stream
     * @return true if successfull
     */
    bool write(EbmlWriter& writer) const;
    
  private:
    T& m_data;
  };
  
  /**
   * Enumerated element is based off integral type
   */
  template <typename E>
  class ElementEnum: public Element<uint32> {
    typedef Element<uint32> super;
  public:
    ElementEnum(id_type ident): super(ident) {}
    ElementEnum(id_type ident, E value): super(ident, static_cast<uint32>(value)) {}
    const E get_value() const { return static_cast<E>(super::get_value()); }
    void set_value(E value) { super::set_value(static_cast<uint32>(value)); }
    bool write(EbmlWriter& writer) const {
      return super::write(writer);
    }
  };
  
  /**
   * Enumerated reference element is based off integral type
   */
  template <typename E>
  class ElementEnumRef: public ElementRef<uint32> {
    typedef ElementRef<uint32> super;
  public:
    ElementEnumRef(id_type ident, E value): super(ident, *reinterpret_cast<uint32*>(&value)) {}
    bool write(EbmlWriter& writer) const {
      return super::write(writer);
    }
  };
  
  inline
  ElementBase::ElementBase(id_type p_id): m_id(p_id), m_next(0) {
  }
  
  inline
  id_type ElementBase::id() const {
    return m_id;
  }
  
  inline
  ElementBase* ElementBase::next() const {
    return m_next;
  }
  
  inline
  size_type ElementBase::size() const {
    const size_type id_size = get_id_size(m_id);
    const size_type inner_size = data_size();
    const size_type encoded_size = get_encoded_size(inner_size);
    const size_type total_size = id_size + encoded_size + inner_size;
    return total_size;
  }
  
  
  inline
  Master::Master(id_type p_id): ElementBase(p_id), m_sub(0) {
  }
  
  inline
  Master& Master::append(ElementBase& element) {
    if (m_sub == 0) {
      m_sub = &element;
    } else {
      ElementBase* esub = m_sub;
      while (esub->m_next != 0) {
        esub = esub->m_next;
      }
      esub->m_next = &element;
    }
    return *this;
  }
  
  inline
  bool Master::write(EbmlWriter& writer) const {
    // Write the id
    const id_type i = id();
    if (!writer.write_id(i)) {
      return false;
    }
    const size_type ds = data_size();
    if (!writer.write_size(ds)) {
      return false;
    }
    ElementBase* esub = m_sub;
    while (esub != 0) {
      if (!esub->write(writer)) {
        return false;
      }
      esub = esub->next();
    }
    return true;
  }
  
  inline
  size_type Master::data_size() const {
    ElementBase* esub = m_sub;
    size_type s = 0;
    while (esub != 0) {
      s += esub->size();
      esub = esub->next();
    }
    return s;
  }
  
  inline
  ElementBase* Master::sub() const {
    return m_sub;
  }
  
  template <typename T>
  Element<T>::Element(id_type p_id): ElementBase(p_id) {
  }
  
  template <typename T>
  Element<T>::Element(id_type p_id, const T& p_data): ElementBase(p_id), m_data(p_data) {
  }
  
  template <typename T>
  bool Element<T>::write(EbmlWriter& writer) const {
    // Write the id
    const id_type i = id();
    if (!writer.write_id(i)) {
      return false;
    }
    const size_type ds = data_size();
    if (!writer.write_size(ds)) {
      return false;
    }
    
    const T& d = get_value();
    if (!writer.write(d)) {
      return false;
    }
    return true;
  }
  
  template <typename T>
  size_type Element<T>::data_size() const {
    return get_data_size(m_data);
  }
  
  template <typename T>
  ElementRef<T>::ElementRef(id_type p_id, T& p_data): ElementBase(p_id), m_data(p_data) {
  }
  
  template <typename T>
  bool ElementRef<T>::write(EbmlWriter& writer) const {
    // Write the id
    const id_type i = id();
    if (!writer.write_id(i)) {
      return false;
    }
    const size_type ds = data_size();
    if (!writer.write_size(ds)) {
      return false;
    }
    
    const T& d = get_value();
    if (!writer.write(d)) {
      return false;
    }
    return true;
  }
  
  template <typename T>
  size_type ElementRef<T>::data_size() const {
    return get_data_size(m_data);
  }
}
