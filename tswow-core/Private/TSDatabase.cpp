/*
 * Copyright (C) 2020 tswow <https://github.com/tswow/>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "TSDatabase.h"
#include "DatabaseEnv.h"
#include "MySQLConnection.h"
#include "TSConsole.h"
#include "PreparedStatement.h"
#include "WorldDatabase.h"
#include "LoginDatabase.h"
#include "CharacterDatabase.h"
#include "QueryResult.h"
#include <memory>
#include <algorithm>

class TC_GAME_API TSDatabaseImpl final : public TSDatabaseResult {
    Field* field = nullptr;
    QueryResult result;
public:
    TSDatabaseImpl(QueryResult result) : TSDatabaseResult()
    {
        this->result = result;
    }

    bool IsValid() final
    {
        return result!=nullptr;
    }

    bool GetRow() final
    {
        if(!result)
        {
            return false;
        }

        if(!field)
        {
            field = result->Fetch();
            return true;
        }

        bool v = result->NextRow();
        if(v)
        {
            field = result->Fetch();
        }
        return v;
    }

#if TRINITY
#define TSGet(TC,AC) TC()
#endif

    TSNumber<uint8> GetUInt8(int index) final { return field[index].TSGet(GetUInt8, uint8); }
    TSNumber<uint16> GetUInt16(int index) final { return field[index].TSGet(GetUInt16,uint16); }
    TSNumber<uint32> GetUInt32(int index) final { return field[index].TSGet(GetUInt32,uint32); }
    TSNumber<uint64> GetUInt64(int index) final { return field[index].TSGet(GetUInt64,uint64); }
    TSGUID GetGUIDNumber(int index) final { return TSGUID(field[index].TSGet(GetUInt64,uint64)); }

    TSNumber<int8> GetInt8(int index) final { return field[index].TSGet(GetInt8,int8); }
    TSNumber<int16> GetInt16(int index) final { return field[index].TSGet(GetInt16,int16); }
    TSNumber<int32> GetInt32(int index) final { return field[index].TSGet(GetInt32,int32); }
    TSNumber<int64> GetInt64(int index) final { return field[index].TSGet(GetInt64,int64); }

    TSNumber<float> GetFloat(int index) final { return field[index].TSGet(GetFloat,float); }
    TSNumber<double> GetDouble(int index) final { return field[index].TSGet(GetDouble,double); }

    std::string GetString(int index) final { 
#if TRINITY
        return field[index].GetString(); 
#endif
    }

    TSArray<uint8> GetBinary(int index) final {
        std::vector<uint8> raw = field[index].GetBinary();
        return TSArray<uint8>(raw);
    }
};

// todo: don't copypaste
class TC_GAME_API TSDatabaseResultPrepared final : public TSDatabaseResult{
    Field * field = nullptr;
    PreparedQueryResult result;
public:
    TSDatabaseResultPrepared(PreparedQueryResult result) : TSDatabaseResult()
    {
        this->result = result;
    }

    bool IsValid() final
    {
        return result != nullptr;
    }

    bool GetRow() final
    {
        if (!result)
        {
            return false;
        }

        if (!field)
        {
            field = result->Fetch();
            return true;
        }

        bool v = result->NextRow();
        if (v)
        {
            field = result->Fetch();
        }
        return v;
    }

    TSNumber<uint8> GetUInt8(int index) final { return field[index].TSGet(GetUInt8,uint8); }
    TSNumber<uint16> GetUInt16(int index) final { return field[index].TSGet(GetUInt16,uint16); }
    TSNumber<uint32> GetUInt32(int index) final { return field[index].TSGet(GetUInt32,uint32); }
    TSNumber<uint64> GetUInt64(int index) final { return field[index].TSGet(GetUInt64,uint64); }
    TSGUID GetGUIDNumber(int index) final { return TSGUID(field[index].TSGet(GetUInt64, uint64)); }

    TSNumber<int8> GetInt8(int index) final { return field[index].TSGet(GetInt8,uint8); }
    TSNumber<int16> GetInt16(int index) final { return field[index].TSGet(GetInt16,uint16); }
    TSNumber<int32> GetInt32(int index) final { return field[index].TSGet(GetInt32,uint32); }
    TSNumber<int64> GetInt64(int index) final { return field[index].TSGet(GetInt64,uint64); }

    TSNumber<float> GetFloat(int index) final { return field[index].TSGet(GetFloat,float); }
    TSNumber<double> GetDouble(int index) final { return field[index].TSGet(GetDouble,double); }

    std::string GetString(int index) final { 
#if TRINITY
        return field[index].GetString(); 
#endif

    }

    TSArray<uint8> GetBinary(int index) final {
        std::vector<uint8> raw = field[index].GetBinary();
        return TSArray<uint8>(raw);
    }
};

std::shared_ptr<TSDatabaseResult> QueryWorld(std::string const& query)
{
    return std::make_shared<TSDatabaseImpl>(WorldDatabase.Query(query.c_str()));
}

std::shared_ptr<TSDatabaseResult> QueryCharacters(std::string const& query)
{
    return std::make_shared<TSDatabaseImpl>(CharacterDatabase.Query(query.c_str()));
}

std::shared_ptr<TSDatabaseResult> QueryAuth(std::string const& query)
{
    return std::make_shared<TSDatabaseImpl>(LoginDatabase.Query(query.c_str()));
}

TSDatabaseConnectionInfo::TSDatabaseConnectionInfo(MySQLConnectionInfo const* info)
    : _info(info)
{}

std::string TSDatabaseConnectionInfo::User() { return _info->user; }
std::string TSDatabaseConnectionInfo::Password() { return _info->password; }
std::string TSDatabaseConnectionInfo::Database() { return _info->database; }
std::string TSDatabaseConnectionInfo::Host() { return _info->host; }
std::string TSDatabaseConnectionInfo::PortOrSocket() { return _info->port_or_socket; }
std::string TSDatabaseConnectionInfo::SSL() { return _info->ssl; }

std::shared_ptr<TSDatabaseConnectionInfo> WorldDatabaseInfo()
{
    return std::make_shared<TSDatabaseConnectionInfo>(WorldDatabase.GetConnectionInfo());
}

std::shared_ptr<TSDatabaseConnectionInfo> CharactersDatabaseInfo()
{
    return std::make_shared<TSDatabaseConnectionInfo>(CharacterDatabase.GetConnectionInfo());
}

std::shared_ptr<TSDatabaseConnectionInfo> AuthDatabaseInfo()
{
    return std::make_shared<TSDatabaseConnectionInfo>(LoginDatabase.GetConnectionInfo());
}

TSPreparedStatementBase::TSPreparedStatementBase(
      PreparedStatementBase* statement
    , TSPreparedStatement* holder
)
    : m_statement(statement)
    , m_holder(holder)
{};
std::shared_ptr<TSDatabaseResult> TSPreparedStatementBase::Send()
{
    return m_holder->Send(this);
}

void TSPreparedStatementBase::SendAsync()
{
    return m_holder->SendAsync(this);
}


std::shared_ptr<TSDatabaseResult> TSPreparedStatementBase::Send(TSWorldDatabaseConnection & con)
{
    return con.Query(this);
}

std::shared_ptr<TSDatabaseResult> TSPreparedStatementBase::Send(TSAuthDatabaseConnection & con)
{
    return con.Query(this);
}

std::shared_ptr<TSDatabaseResult> TSPreparedStatementBase::Send(TSCharactersDatabaseConnection & con)
{
    return con.Query(this);
}

TSPreparedStatementBase* TSPreparedStatementBase::SetNull(const uint8 index)
{
#if TRINITY
    m_statement->setNull(index);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetUInt8(const uint8 index, const uint8 value)
{
#if TRINITY
    m_statement->setUInt8(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetInt8(const uint8 index, const int8 value)
{
#if TRINITY
    m_statement->setInt8(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetUInt16(const uint8 index, const uint16 value)
{
#if TRINITY
    m_statement->setUInt16(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetInt16(const uint8 index, const int16 value)
{
#if TRINITY
    m_statement->setInt16(index, value);
#endif 
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetUInt32(const uint8 index, const uint32 value)
{
#if TRINITY
    m_statement->setUInt32(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetInt32(const uint8 index, const int32 value)
{
#if TRINITY
    m_statement->setInt32(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetUInt64(const uint8 index, const uint64 value)
{
#if TRINITY
    m_statement->setUInt64(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetGUIDNumber(const uint8 index, const TSGUID value)
{
#if TRINITY
    m_statement->setUInt64(index, value.asGUID().GetRawValue());
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetInt64(const uint8 index, const int64 value)
{
#if TRINITY
    m_statement->setInt64(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetFloat(const uint8 index, const float value)
{
#if TRINITY
    m_statement->setFloat(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetDouble(const uint8 index, const double value)
{
#if TRINITY
    m_statement->setDouble(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetString(const uint8 index, std::string const& value)
{
#if TRINITY
    m_statement->setString(index, value);
#endif
    return this;
}

TSPreparedStatementBase* TSPreparedStatementBase::SetBinary(const uint8 index, TSArray<uint8> value)
{
    m_statement->setBinary(index, *value.vec.get());
    return this;
}

TSPreparedStatementBase TSPreparedStatement::Create()
{
    return TSPreparedStatementBase(new PreparedStatementBase(0,m_paramCount), this);
}

std::shared_ptr<TSDatabaseResult> TSPreparedStatementWorld::Send(TSPreparedStatementBase* stmnt)
{
#if TRINITY
    auto ptr = std::make_shared<TSDatabaseResultPrepared>(WorldDatabase.QueryCustomStatement(m_id, stmnt->m_statement));
    delete stmnt->m_statement;
    return ptr;
#endif
}

std::shared_ptr<TSDatabaseResult> TSPreparedStatementCharacters::Send(TSPreparedStatementBase* stmnt)
{
#if TRINITY
    auto ptr = std::make_shared<TSDatabaseResultPrepared>(CharacterDatabase.QueryCustomStatement(m_id, stmnt->m_statement));
    delete stmnt->m_statement;
    return ptr;
#endif
}

std::shared_ptr<TSDatabaseResult> TSPreparedStatementAuth::Send(TSPreparedStatementBase* stmnt)
{
#if TRINITY
    auto ptr = std::make_shared<TSDatabaseResultPrepared>(LoginDatabase.QueryCustomStatement(m_id, stmnt->m_statement));
    delete stmnt->m_statement;
    return ptr;
#endif
}

void TSPreparedStatementWorld::SendAsync(TSPreparedStatementBase* stmnt)
{
    WorldDatabase.QueryCustomStatementAsync(m_id, stmnt->m_statement);
}

void TSPreparedStatementCharacters::SendAsync(TSPreparedStatementBase* stmnt)
{
    CharacterDatabase.QueryCustomStatementAsync(m_id, stmnt->m_statement);
}

void TSPreparedStatementAuth::SendAsync(TSPreparedStatementBase* stmnt)
{
    LoginDatabase.QueryCustomStatementAsync(m_id, stmnt->m_statement);
}

TSPreparedStatement::TSPreparedStatement(std::string const& sql, uint32 id)
    : m_id(id)
    , m_paramCount(std::count(sql.begin(),sql.end(),'?'))
{

}

TSPreparedStatementWorld::TSPreparedStatementWorld(std::string const& sql)
#if TRINITY
    : TSPreparedStatement(sql, WorldDatabase.PrepareCustomStatement(sql))
#endif
{}

TSPreparedStatementCharacters::TSPreparedStatementCharacters(std::string const& sql)
#if TRINITY
    : TSPreparedStatement(sql, CharacterDatabase.PrepareCustomStatement(sql))
#endif
{}

TSPreparedStatementAuth::TSPreparedStatementAuth(std::string const& sql)
#if TRINITY
    : TSPreparedStatement(sql, LoginDatabase.PrepareCustomStatement(sql))
#endif
{}

TC_GAME_API TSPreparedStatementWorld PrepareWorldQuery(std::string const& query)
{
    return TSPreparedStatementWorld(query);
}

TC_GAME_API TSPreparedStatementCharacters PrepareCharactersQuery(std::string const& query)
{
    return TSPreparedStatementCharacters(query);
}

TC_GAME_API TSPreparedStatementAuth PrepareAuthQuery(std::string const& query)
{
    return TSPreparedStatementAuth(query);
}

static QueryResult ResultFromSet(ResultSet* res)
{
    if (!res || !res->GetRowCount() || !res->NextRow())
    {
        delete res;
        return QueryResult(nullptr);
    }
    return QueryResult(res);
}

static PreparedQueryResult PreparedResultFromSet(PreparedResultSet* res)
{
    if (!res || !res->GetRowCount())
    {
        delete res;
        return PreparedQueryResult(nullptr);
    }
    return PreparedQueryResult(res);
}

// WorldDBConnection

TSWorldDatabaseConnection::TSWorldDatabaseConnection(WorldDatabaseConnection* connection)
    : m_connection(connection) {}

std::shared_ptr<TSDatabaseResult> TSWorldDatabaseConnection::Query(std::string const& sql)
{
    return std::make_shared<TSDatabaseImpl>(ResultFromSet(m_connection->Query(sql.c_str())));
}

std::shared_ptr<TSDatabaseResult> TSWorldDatabaseConnection::Query(TSPreparedStatementBase * stmnt)
{
#if TRINITY
    auto res = std::make_shared<TSDatabaseResultPrepared>(
        WorldDatabase.QueryCustomStatement(
            stmnt->m_holder->m_id, stmnt->m_statement, m_connection
        ));
    delete stmnt->m_statement;
    return res;
#endif
}

void TSWorldDatabaseConnection::Unlock()
{
#if TRINITY
    m_connection->Unlock();
#endif
}


// AuthDBConnection

TSAuthDatabaseConnection::TSAuthDatabaseConnection(LoginDatabaseConnection* connection)
    : m_connection(connection) {}

std::shared_ptr<TSDatabaseResult> TSAuthDatabaseConnection::Query(std::string const& sql)
{
    return std::make_shared<TSDatabaseImpl>(ResultFromSet(m_connection->Query(sql.c_str())));
}

std::shared_ptr<TSDatabaseResult> TSAuthDatabaseConnection::Query(TSPreparedStatementBase* stmnt)
{
#if TRINITY
    auto res = std::make_shared<TSDatabaseResultPrepared>(
        LoginDatabase.QueryCustomStatement(
            stmnt->m_holder->m_id, stmnt->m_statement, m_connection
        ));
    delete stmnt->m_statement;
    return res;
#endif
}

void TSAuthDatabaseConnection::Unlock()
{
#if TRINITY
    m_connection->Unlock();
#endif
}


// CharDBConnection

TSCharactersDatabaseConnection::TSCharactersDatabaseConnection(CharacterDatabaseConnection* connection)
    : m_connection(connection) {}

std::shared_ptr<TSDatabaseResult> TSCharactersDatabaseConnection::Query(std::string const& sql)
{
    return std::make_shared<TSDatabaseImpl>(ResultFromSet(m_connection->Query(sql.c_str())));
}

std::shared_ptr<TSDatabaseResult> TSCharactersDatabaseConnection::Query(TSPreparedStatementBase* stmnt)
{
#if TRINITY
    auto res = std::make_shared<TSDatabaseResultPrepared>(
        CharacterDatabase.QueryCustomStatement(
            stmnt->m_holder->m_id, stmnt->m_statement, m_connection
        ));
    delete stmnt->m_statement;
    return res;
#endif
}

void TSCharactersDatabaseConnection::Unlock()
{
#if TRINITY
    m_connection->Unlock();
#endif
}

TC_GAME_API TSWorldDatabaseConnection GetWorldDBConnection()
{
#if TRINITY
    return TSWorldDatabaseConnection(WorldDatabase.GetFreeConnection());
#endif
}

TC_GAME_API TSAuthDatabaseConnection GetAuthDBConnection()
{
#if TRINITY
    return TSAuthDatabaseConnection(LoginDatabase.GetFreeConnection());
#endif
}

TC_GAME_API TSCharactersDatabaseConnection GetCharactersDBConnection()
{
#if TRINITY
    return TSCharactersDatabaseConnection(CharacterDatabase.GetFreeConnection());
#endif
}

TC_GAME_API void QueryWorldAsync(std::string const& query)
{
    WorldDatabase.AsyncQuery(query.c_str());
}

TC_GAME_API void QueryCharactersAsync(std::string const& query)
{
    CharacterDatabase.AsyncQuery(query.c_str());
}

TC_GAME_API void QueryAuthAsync(std::string const& query)
{
    LoginDatabase.AsyncQuery(query.c_str());
}
