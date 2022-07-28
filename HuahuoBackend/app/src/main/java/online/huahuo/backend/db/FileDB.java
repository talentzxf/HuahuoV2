package online.huahuo.backend.db;

import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.RequiredArgsConstructor;
import org.springframework.beans.factory.annotation.Required;

import javax.persistence.*;

@Entity
@Data
@NoArgsConstructor
public class FileDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    private String name;
    private String type;

    @Lob
    private byte[] data;

    public FileDB(String name, String type, byte[] data){
        this.name = name;
        this.type = type;
        this.data = data;
    }
}
