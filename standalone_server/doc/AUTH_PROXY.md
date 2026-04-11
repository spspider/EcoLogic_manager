# Auth Proxy Integration with Grafana

## 🎯 **Overview**

EcoLogic Manager теперь использует **Auth Proxy** для автоматического создания пользователей в Grafana. Это устраняет проблемы с API разрешениями и работает более надежно.

## 🔧 **Как это работает**

1. **MySQL пользователь** создается через EcoLogic Manager
2. **Grafana пользователь** создается автоматически при первом входе 
3. **Auth Proxy** передает заголовок `X-WEBAUTH-USER` 
4. **Grafana** доверяет этому заголовку и создает пользователя

## 🚀 **Endpoints**

### Create & Auto-Login
```http
GET /user-management/api/grafana-login/{username}
```
Создает MySQL пользователя и автоматически логинит в Grafana.

### Bulk Sync via Auth Proxy
```http
POST /user-management/api/sync-auth-proxy
```
Синхронизирует всех MySQL пользователей в Grafana через Auth Proxy.

## ⚙️ **Configuration**

### Grafana Environment Variables
```yaml
GF_AUTH_PROXY_ENABLED=true
GF_AUTH_PROXY_HEADER_NAME=X-WEBAUTH-USER  
GF_AUTH_PROXY_HEADER_PROPERTY=username
GF_AUTH_PROXY_AUTO_SIGN_UP=true
```

### Nginx Configuration
```nginx
location /graf/ {
    proxy_set_header X-WEBAUTH-USER $remote_user;
    proxy_pass http://grafana:3000/graf/;
    # ... other headers
}
```

## 🔍 **How Auth Proxy Works**

1. **EcoLogic Manager** проверяет пользователя в MySQL
2. **Делает HTTP запрос** к Grafana с заголовком `X-WEBAUTH-USER` 
3. **Grafana получает заголовок** и автоматически:
   - Создает пользователя (если не существует)  
   - Логинит пользователя
   - Назначает роль по умолчанию

## ✅ **Advantages of Auth Proxy**

- ❌ **Не нужны API разрешения** (users:create)
- ✅ **Автоматическое создание пользователей**  
- ✅ **Надежная интеграция**
- ✅ **Простая настройка**
- ✅ **Поддерживает SSO**

## 🐛 **Troubleshooting**

### User not created in Grafana
1. Check Grafana logs: `docker logs grafana`
2. Verify `GF_AUTH_PROXY_ENABLED=true`
3. Verify `X-WEBAUTH-USER` header is sent

### Auth Proxy not working  
1. Check Nginx configuration
2. Verify proxy headers are forwarded
3. Test direct Grafana access

## 📝 **Migration from API Method**

Old API method (deprecated):
```python
# ❌ Old way - requires users:create permission
create_user_in_grafana(username, password, email)
```

New Auth Proxy method (recommended):  
```python
# ✅ New way - automatic via headers
headers = {'X-WEBAUTH-USER': username}
requests.get(f"{GRAFANA_URL}/api/user", headers=headers)
```