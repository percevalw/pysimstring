#include <string>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <errno.h>
#include <pysimstring/simstring/simstring.h>
#include <codecvt>
#include <cassert>

#include "export.h"

#define UTF16   "UTF-16LE"
#define UTF32   "UTF-32LE"

#if defined (WIN32)
#define __SIZEOF_WCHAR_T__ 2
#endif

#if defined(__APPLE__) || defined(WIN32)
    typedef wchar_t CHAR_TYPE_16;
#else
    typedef char16_t CHAR_TYPE_16;
#endif
#if defined(__APPLE__) || defined(WIN32)
    typedef wchar_t CHAR_TYPE_32;
#else
    typedef char32_t CHAR_TYPE_32;
#endif

typedef std::wstring_convert<std::codecvt_utf8<CHAR_TYPE_16>,CHAR_TYPE_16> CONVERT_16;
typedef std::wstring_convert<std::codecvt_utf8<CHAR_TYPE_32>,CHAR_TYPE_32> CONVERT_32;


int translate_measure(int measure)
{
    switch (measure) {
    case exact:
        return simstring::exact;
    case dice:
        return simstring::dice;
    case cosine:
        return simstring::cosine;
    case jaccard:
        return simstring::jaccard;
    case overlap:
        return simstring::overlap;
    }
    throw std::invalid_argument("Unknown similarity measure specified");
}



typedef simstring::ngram_generator ngram_generator_type;
typedef simstring::writer_base<std::string, ngram_generator_type> writer_type;
typedef simstring::writer_base<std::wstring, ngram_generator_type> uwriter_type;
typedef simstring::reader reader_type;

writer::writer(const char *filename, int n, bool be, bool unicode)
    : m_dbw(NULL), m_gen(NULL), m_unicode(unicode)
{
    ngram_generator_type *gen = new ngram_generator_type(n, be);
    if (unicode) {
        uwriter_type *dbw = new uwriter_type(*gen, filename);
        if (dbw->fail()) {
            std::string message = dbw->error();
            delete dbw;
            delete gen;
            throw std::invalid_argument(message);
        }
        m_dbw = dbw;
        m_gen = gen;
    } else {
        writer_type *dbw = new writer_type(*gen, filename);
        if (dbw->fail()) {
            std::string message = dbw->error();
            delete dbw;
            delete gen;
            throw std::invalid_argument(message);
        }
        m_dbw = dbw;
        m_gen = gen;
    }
}

writer::~writer()
{
    if (m_unicode) {
        uwriter_type* dbw = reinterpret_cast<uwriter_type*>(m_dbw);
    ngram_generator_type* gen = reinterpret_cast<ngram_generator_type*>(m_gen);
    
    dbw->close();
    if (dbw->fail()) {
        std::string message = dbw->error();
        delete dbw;
        delete gen;
        throw std::runtime_error(message);
    }
    delete dbw;
    delete gen;

    } else {
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    ngram_generator_type* gen = reinterpret_cast<ngram_generator_type*>(m_gen);
    
    dbw->close();
    if (dbw->fail()) {
        std::string message = dbw->error();
        delete dbw;
        delete gen;
        throw std::runtime_error(message);
    }
    delete dbw;
    delete gen;
    }
}

void writer::insert(const char *string)
{
    if (m_unicode) {
        uwriter_type* dbw = reinterpret_cast<uwriter_type*>(m_dbw);
        std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> convert;
        std::wstring str = convert.from_bytes(std::string(string));

        dbw->insert(str);

        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }
    } else {
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
        dbw->insert(string);
        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }
    }
}

void writer::close()
{
    if (m_unicode) {
        uwriter_type* dbw = reinterpret_cast<uwriter_type*>(m_dbw);
        dbw->close();
        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }

    } else {
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
        dbw->close();
        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }
    }
}



reader::reader(const char *filename)
    : m_dbr(NULL), measure(cosine), threshold(0.7)
{
    reader_type *dbr = new reader_type;

    if (!dbr->open(filename)) {
        delete dbr;
        throw std::invalid_argument("Failed to open the database");
    }

    m_dbr = dbr;
}

reader::~reader()
{
    this->close();
    delete reinterpret_cast<reader_type*>(m_dbr);
}

template <class insert_iterator_type>
void retrieve_thru(
    reader_type& dbr,
    const std::string& query,
    int measure,
    double threshold,
    insert_iterator_type ins
    )
{
    switch (measure) {
    case exact:
        dbr.retrieve<simstring::measure::exact>(query, threshold, ins);
        break;
    case dice:
        dbr.retrieve<simstring::measure::dice>(query, threshold, ins);
        break;
    case cosine:
        dbr.retrieve<simstring::measure::cosine>(query, threshold, ins);
        break;
    case jaccard:
        dbr.retrieve<simstring::measure::jaccard>(query, threshold, ins);
        break;
    case overlap:
        dbr.retrieve<simstring::measure::overlap>(query, threshold, ins);
        break;
    }
}

template <class Converter, class char_type, class insert_iterator_type>
void retrieve_iconv(
    reader_type& dbr,
    const std::string& query,
    int measure,
    double threshold,
    insert_iterator_type ins
    )
{
    typedef std::basic_string<char_type> string_type;
    typedef std::vector<string_type> strings_type;

    // Translate the character encoding of the query string from UTF-8 to the target encoding.
    Converter convert;
    string_type qstr = convert.from_bytes(query);

    strings_type xstrs;
    switch (measure) {
    case exact:
        dbr.retrieve<simstring::measure::exact>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case dice:
        dbr.retrieve<simstring::measure::dice>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case cosine:
        dbr.retrieve<simstring::measure::cosine>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case jaccard:
        dbr.retrieve<simstring::measure::jaccard>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case overlap:
        dbr.retrieve<simstring::measure::overlap>(qstr, threshold, std::back_inserter(xstrs));
        break;
    }

    // Translate back the character encoding of retrieved strings into UTF-8.
    for (typename strings_type::const_iterator it = xstrs.begin();it != xstrs.end();++it) {
        std::string dst = convert.to_bytes(*it);
        *ins = dst;
    }
}

#if defined(__APPLE__) || defined(WIN32)
#include <cassert>
#endif

std::vector<std::string> reader::retrieve(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    std::vector<std::string> ret;

    switch (dbr.char_size()) {
        case 1:
            retrieve_thru(dbr, query, this->measure, this->threshold, std::back_inserter(ret));
            break;
        case 2:
            retrieve_iconv<CONVERT_16, CHAR_TYPE_16>(dbr, query, this->measure, this->threshold, std::back_inserter(ret));
            break;
        case 4:
            retrieve_iconv<CONVERT_32, CHAR_TYPE_32>(dbr, query, this->measure, this->threshold, std::back_inserter(ret));
            break;
    }

    return ret;
}

bool reader::check(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);

    if (dbr.char_size() == 1) {
        std::string qstr = query;
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    } else if (dbr.char_size() == 2) {
        assert(__SIZEOF_WCHAR_T__ == 2);
        CONVERT_16 convert;
        std::basic_string<CHAR_TYPE_16> qstr = convert.from_bytes(query);
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    } else if (dbr.char_size() == 4) {
        assert(__SIZEOF_WCHAR_T__ == 4);
        CONVERT_32 convert;
        std::basic_string<CHAR_TYPE_32> qstr = convert.from_bytes(query);
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    }
    
    return false;
}

void reader::close()
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    dbr.close();
}
