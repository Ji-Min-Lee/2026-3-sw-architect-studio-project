# PlantUML 렌더링 테스트

VS Code에서 PlantUML 플러그인 설치 후 아래 다이어그램이 렌더링되는지 확인.

- [PlantUML (jebbs)](https://marketplace.visualstudio.com/items?itemName=jebbs.plantuml)
- [PlantUML Markdown Preview](https://marketplace.visualstudio.com/items?itemName=yss-tazawa.plantuml-markdown-preview)

---

## 1. Sequence Diagram

```plantuml
@startuml
Client -> Server : HTTP Request
Server -> DB : Query
DB --> Server : Result
Server --> Client : HTTP Response
@enduml
```

---

## 2. Class Diagram

```plantuml
@startuml
class Animal {
  +name: String
  +speak(): void
}
class Dog extends Animal {
  +fetch(): void
}
class Cat extends Animal {
  +purr(): void
}
@enduml
```

---

## 3. Component Diagram

```plantuml
@startuml
package "Frontend" {
  [Web App]
}
package "Backend" {
  [API Server]
  [Auth Service]
}
database "DB" {
  [PostgreSQL]
}
[Web App] --> [API Server] : REST
[API Server] --> [Auth Service] : gRPC
[API Server] --> [PostgreSQL] : SQL
@enduml
```

---

## 4. State Diagram

```plantuml
@startuml
[*] --> Idle
Idle --> Processing : request
Processing --> Success : ok
Processing --> Error : fail
Success --> [*]
Error --> Idle : retry
@enduml
```
