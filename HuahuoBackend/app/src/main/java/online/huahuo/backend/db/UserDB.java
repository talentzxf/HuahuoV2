package online.huahuo.backend.db;

import lombok.Data;
import lombok.extern.java.Log;

import javax.persistence.*;

@Entity
@Data
@Log
@Table(name = "USERACCOUNTS", indexes = @Index(columnList = "identifier"))
public class UserDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(unique = true)
    private String username; // Might be a phone number, might be email, might be external ref. but need to be unique.

    @Column(name = "password_hash")
    private String password;

    private String nickname;

    // Huahuo has following foles:
    // CREATOR/CREATOR/READER/ANONYMOUS
    private String roles;
}
